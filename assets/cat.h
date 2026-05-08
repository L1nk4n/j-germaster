#ifndef CAT_H
#define CAT_H

#include "flatmodel.h"
#include "../shader/shader.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

class Cat {
public:
  Cat(glm::vec3 position, FlatModel& model, Shader& jaegerShader) : shaderRef(jaegerShader), modelRef(model) {
    worldPosition = position;
    shaderRef = jaegerShader;
  }

  ~Cat() {
  }

  void lookAt(glm::vec3 target) {
    lookingAt = target;
  }

  void update() {
    glm::mat4 catModel = glm::mat4(1.0f);

    glm::vec3 dir = lookingAt - worldPosition;
    dir.y = 0.0f; // flatten to x,z plane
    dir = glm::normalize(dir);

    float angle = atan2(dir.x, dir.z);

    catModel = glm::translate(catModel, worldPosition);
    catModel = glm::rotate(catModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    catModel = glm::rotate(catModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    shaderRef.setMat4("model", catModel);
    modelRef.draw();
  }
private:
  glm::vec3 worldPosition = glm::vec3(0.0f, 1.5f, 0.0f);
  glm::vec3 lookingAt = glm::vec3(1.0f, 1.0f, 1.0f);
  FlatModel& modelRef;
  Shader& shaderRef;
};

#endif
