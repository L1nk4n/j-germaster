#ifndef GRID_H
#define GRID_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include <vector>
#include <iostream>

class Ground {
public:
    Ground(float size, int divisions, float yLevel, const char* texturePath) {
      std::vector<float> vertices;

      float half = size * 0.5f;
      float step = size / (float)divisions;
      float tilingScale = 0.1f;
      for (int i = 0; i <= divisions; i++) {
        for (int j = 0; j <= divisions; j++) {
          float x = -half + i * step;
          float z = -half + j * step;
          vertices.push_back(x);
          vertices.push_back(yLevel);
          vertices.push_back(z);
          vertices.push_back((float)i * tilingScale);
          vertices.push_back((float)j * tilingScale);
          vertices.push_back(0.0f);
          vertices.push_back(1.0f);
          vertices.push_back(0.0f);
        }
      }
      std::vector<unsigned int> indices;
      unsigned int row = divisions + 1;
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

      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);
      
      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

      const int stride = 8 * sizeof(float);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
      glEnableVertexAttribArray(2);

      glBindVertexArray(0);

      glGenTextures(1, &textures);
      glBindTexture(GL_TEXTURE_2D, textures);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     int width, height, nrChannels;
     unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
     if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
     } else {
        std::cout << "Failed to load ground texture: " << texturePath << std::endl;
     }
    stbi_image_free(data);

    indexCount = (GLsizei)indices.size();

    }

    ~Ground() {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);
      glDeleteTextures(1, &textures);
    }

    void draw() const {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textures);
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int textures;
    int indexCount;
};

#endif
