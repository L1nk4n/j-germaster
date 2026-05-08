#ifndef FLAT_MODEL_H
#define FLAT_MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include <vector>
#include <iostream>

class FlatModel {
public:
    FlatModel(float size, float yLevel, const char* texturePath) {
      std::vector<float> vertices;
      for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
          float x = (j == 0 ? -1.0f : 1.0f) * size * 0.5f;
          float z = (i == 0 ? -1.0f : 1.0f) * size * 0.5f;

          vertices.push_back(x);
          vertices.push_back(yLevel);
          vertices.push_back(z);
          vertices.push_back((float)j);
          vertices.push_back((float)i);
          vertices.push_back(0.0f);
          vertices.push_back(1.0f);
          vertices.push_back(0.0f);
        }
      }

      std::vector<unsigned int> indices;
      unsigned int bl = 3;
      unsigned int br = 2;
      unsigned int tl = 1;
      unsigned int tr = 0;

      indices.push_back(bl);
      indices.push_back(br);
      indices.push_back(tr);

      indices.push_back(bl);
      indices.push_back(tr);
      indices.push_back(tl);

      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);
      
      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

      const GLsizei stride = 8 * sizeof(float);
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
     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

    // Also update destructor to guard against deleting 0
    ~FlatModel() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        if (textures) glDeleteTextures(1, &textures);
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
