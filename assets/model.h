#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

class Model {
public:
    Model(const char* objPath, const char* texturePath) {
        loadOBJ(objPath);
        setupBuffers();
        loadTexture(texturePath);
    }

    ~Model() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteTextures(1, &texture);
    }

    void draw() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0, texture = 0;
    GLsizei indexCount = 0;

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
    }

    void loadTexture(const char* path) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, channels;
        unsigned char* data = stbi_load(path, &w, &h, &channels, 0);
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load model texture: " << path << std::endl;
        }
        stbi_image_free(data);
    }
};

#endif
