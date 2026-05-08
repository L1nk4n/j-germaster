#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

struct Character {
  unsigned int TextureID;
  glm::ivec2 Size;
  glm::ivec2 Bearing;
  unsigned int Advance;
};

class TextRenderer {
  public:
    TextRenderer(const std::string& fontPath, unsigned int fontSize);
    ~TextRenderer();

    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
  private:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
};

#endif
