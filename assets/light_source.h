#ifndef LIGHTS_H
#define LIGHTS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

class LightSource {
public:
    LightSource(const char* objPath, glm::vec3 position, glm::vec3 relative_light_position) {
        worldPosition = position;
        relLightPosition = relative_light_position;
        loadOBJ(objPath);
        setupBuffers();
    }

    ~LightSource() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    glm::vec3 lightPos() {
        return worldPosition + relLightPosition;
    }

    glm::vec3 pos() {
        return worldPosition;
    }

    void draw() const {
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
private:
    unsigned int lightVAO;
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    GLsizei indexCount = 0;
    glm::vec3 worldPosition;
    glm::vec3 relLightPosition;

    // Final interleaved vertex data: [px,py,pz, u,v, nx,ny,nz] per vertex
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Key for deduplicating combined vertices
    struct VertexKey {
        int posIdx;
        int uvIdx;
        int normIdx;
        bool operator==(const VertexKey& o) const {
            return posIdx == o.posIdx && uvIdx == o.uvIdx && normIdx == o.normIdx;
        }
    };
    struct VertexKeyHash {
        size_t operator()(const VertexKey& k) const {
            // Simple hash combine; fine for our scale
            return std::hash<int>()(k.posIdx) ^
                   (std::hash<int>()(k.uvIdx) << 1) ^
                   (std::hash<int>()(k.normIdx) << 2);
        }
    };

    void loadOBJ(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "Failed to open OBJ file: " << path << std::endl;
            return;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::unordered_map<VertexKey, unsigned int, VertexKeyHash> seen;

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream s(line);
            std::string tag;
            s >> tag;

            if (tag == "v") {
                glm::vec3 p; s >> p.x >> p.y >> p.z;
                positions.push_back(p);
            } else if (tag == "vt") {
                glm::vec2 t; s >> t.x >> t.y;
                uvs.push_back(t);
            } else if (tag == "vn") {
                glm::vec3 n; s >> n.x >> n.y >> n.z;
                normals.push_back(n);
            } else if (tag == "f") {
                for (int i = 0; i < 3; i++) {
                    std::string token;
                    s >> token;

                    for (char& c : token) if (c == '/') c = ' ';
                    std::istringstream ts(token);

                    int pi, ti, ni;
                    ts >> pi >> ti >> ni;

                    VertexKey key{ pi - 1, ti - 1, ni - 1 };

                    auto it = seen.find(key);
                    if (it != seen.end()) {
                        indices.push_back(it->second);
                    } else {
                        unsigned int newIdx = static_cast<unsigned int>(vertices.size() / 8);

                        const glm::vec3& p = positions[key.posIdx];
                        const glm::vec2& t = uvs[key.uvIdx];
                        const glm::vec3& n = normals[key.normIdx];

                        vertices.push_back(p.x);
                        vertices.push_back(p.y);
                        vertices.push_back(p.z);
                        vertices.push_back(t.x);
                        vertices.push_back(t.y);
                        vertices.push_back(n.x);
                        vertices.push_back(n.y);
                        vertices.push_back(n.z);

                        seen[key] = newIdx;
                        indices.push_back(newIdx);
                    }
                }
            }
        }

        indexCount = static_cast<GLsizei>(indices.size());
        std::cout << "Loaded " << path
                  << " — vertices: " << vertices.size() / 8
                  << ", indices: " << indices.size() << std::endl;
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(float),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(), GL_STATIC_DRAW);

        // Layout: position (3), uv (2), normal (3) — stride = 8 floats
        const GLsizei stride = 8 * sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        vertices.clear(); vertices.shrink_to_fit();
        indices.clear();  indices.shrink_to_fit();

        glGenVertexArrays(1, &lightVAO);
        glBindVertexArray(lightVAO);
        // we only need to bind to the VBO, the container's VBO's data already contains the data.
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // set the vertex attribute 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
};

#endif
