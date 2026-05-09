#ifndef TERRAIN_H
#define TERRAIN_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

class PerlinNoise {
  public:
    PerlinNoise(unsigned int seed = 1337) {
      p.resize(512);
      std::vector<int> perm(256);
      for (int i = 0; i < 256; i++) perm[i] = i;

      unsigned int s = seed;
      for (int i = 255; i > 0; i--) {
        s = s * 1664525u + 1013904223u;
        int j = s % (i + 1);
        std::swap(perm[i], perm[j]);
      }
      for (int i = 0; i < 512; i++) p[i] = perm[i & 255];
    }

    float noise(float x, float y) const {
      int X = (int)std::floor(x) & 255;
      int Y = (int)std::floor(y) & 255;
      x -= std::floor(x);
      y -= std::floor(y);
      float u = fade(x);
      float v = fade(y);
      int A = p[X] + Y;
      int B = p[X + 1] + Y;
      return lerp(v,
            lerp(u, grad(p[A],     x,     y),
                    grad(p[B],     x - 1, y)),
            lerp(u, grad(p[A + 1], x,     y - 1),
                    grad(p[B + 1], x - 1, y - 1)));
    }

    float fbm(float x, float y, int octaves, float lacunarity, float gain) const {
        float sum = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float norm = 0.0f;
        for (int i = 0; i < octaves; i++) {
            sum += amplitude * noise(x * frequency, y * frequency);
            norm += amplitude;
            amplitude *= gain;
            frequency *= lacunarity;
        }
        return sum / norm;
    }
private:
    std::vector<int> p;
    static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static float lerp(float t, float a, float b) { return a + t * (b - a); }
    static float grad(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
};


class Terrain {
public:
    // Big terrain features: smaller value = bigger / more spread-out mountains
    static constexpr float CONTINENT_SCALE = 0.012f;
    // Small detail noise (subtle ground variation)
    static constexpr float DETAIL_SCALE    = 0.06f;
    // How tall mountains can get
    static constexpr float PEAK_HEIGHT     = 28.0f;
    // Detail bumps on flat ground
    static constexpr float DETAIL_HEIGHT   = 0.8f;
    // Threshold above which terrain becomes "mountain"; below = flat
    static constexpr float MOUNTAIN_START  = 0.35f;
    static constexpr float VALLEY_FLOOR    = 0.0f;

    static constexpr int   OCTAVES         = 5;
    static constexpr float LACUNARITY      = 2.0f;
    static constexpr float GAIN            = 0.5f;
    static constexpr float TILING_SCALE    = 0.15f;

    Terrain(float size, int divisions, float yLevel,
            const char* grassPath, const char* rockPath, const char* snowPath,
            unsigned int seed = 1337)
        : noiseGen(seed), detailGen(seed + 9999), baseY(yLevel)
    {
        const int row = divisions + 1;
        std::vector<glm::vec3> positions(row * row);
        std::vector<glm::vec2> uvs(row * row);
        std::vector<glm::vec3> normals(row * row, glm::vec3(0.0f));

        float half = size * 0.5f;
        float step = size / (float)divisions;

        for (int i = 0; i <= divisions; i++) {
            for (int j = 0; j <= divisions; j++) {
                float x = -half + i * step;
                float z = -half + j * step;
                float y = sampleHeight(x, z);

                int idx = i * row + j;
                positions[idx] = glm::vec3(x, y, z);
                uvs[idx] = glm::vec2((float)i * TILING_SCALE,
                                     (float)j * TILING_SCALE);
            }
        }

        std::vector<unsigned int> indices;
        indices.reserve(divisions * divisions * 6);
        for (int i = 0; i < divisions; i++) {
            for (int j = 0; j < divisions; j++) {
                unsigned int bl = i * row + j;
                unsigned int br = i * row + (j + 1);
                unsigned int tl = (i + 1) * row + j;
                unsigned int tr = (i + 1) * row + (j + 1);
                indices.push_back(bl);
                indices.push_back(br);
                indices.push_back(tr);
                indices.push_back(bl);
                indices.push_back(tr);
                indices.push_back(tl);
            }
        }

        for (size_t k = 0; k < indices.size(); k += 3) {
            unsigned int a = indices[k];
            unsigned int b = indices[k + 1];
            unsigned int c = indices[k + 2];
            glm::vec3 e1 = positions[b] - positions[a];
            glm::vec3 e2 = positions[c] - positions[a];
            glm::vec3 n  = glm::normalize(glm::cross(e1, e2));
            normals[a] += n;
            normals[b] += n;
            normals[c] += n;
        }
        for (auto& n : normals) n = glm::normalize(n);

        std::vector<float> vertices;
        vertices.reserve(positions.size() * 8);
        for (size_t k = 0; k < positions.size(); k++) {
            vertices.push_back(positions[k].x);
            vertices.push_back(positions[k].y);
            vertices.push_back(positions[k].z);
            vertices.push_back(uvs[k].x);
            vertices.push_back(uvs[k].y);
            vertices.push_back(normals[k].x);
            vertices.push_back(normals[k].y);
            vertices.push_back(normals[k].z);
        }

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

        const GLsizei stride = 8 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                              (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        grassTex = loadTexture(grassPath);
        rockTex  = loadTexture(rockPath);
        snowTex  = loadTexture(snowPath);

        indexCount = (GLsizei)indices.size();
    }

    ~Terrain() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteTextures(1, &grassTex);
        glDeleteTextures(1, &rockTex);
        glDeleteTextures(1, &snowTex);
    }

    void draw() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rockTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, snowTex);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    float getHeightAt(float x, float z) const {
        return sampleHeight(x, z);
    }

private:
    PerlinNoise noiseGen;
    PerlinNoise detailGen;
    float baseY;
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int grassTex = 0, rockTex = 0, snowTex = 0;
    GLsizei indexCount = 0;

    // Two-layer terrain: low-frequency "continent" noise decides where
    // mountains are. Below the mountain threshold = flat valley floor.
    // This gives us walkable ground with dramatic peaks as features.
    float sampleHeight(float x, float z) const {
        float c = noiseGen.fbm(x * CONTINENT_SCALE, z * CONTINENT_SCALE,
                               OCTAVES, LACUNARITY, GAIN);
        c = c * 0.5f + 0.5f; // remap to [0, 1]

        float d = detailGen.noise(x * DETAIL_SCALE, z * DETAIL_SCALE);
        float h = baseY + VALLEY_FLOOR + d * DETAIL_HEIGHT;

        if (c > MOUNTAIN_START) {
            float t = (c - MOUNTAIN_START) / (1.0f - MOUNTAIN_START);
            t = t * t * (3.0f - 2.0f * t); // smoothstep easing
            h += t * PEAK_HEIGHT;
        }
        return h;
    }

    static unsigned int loadTexture(const char* path) {
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, ch;
        unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
        if (data) {
            GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt,
                         GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load terrain texture: " << path << std::endl;
        }
        stbi_image_free(data);
        return tex;
    }
};

#endif
