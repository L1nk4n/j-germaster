#ifndef RAYCASTING_H
#define RAYCASTING_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../assets/view_camera.h"


class Raycasting {
  public:
    glm::vec3 rayOrigin;
    glm::vec3 rayDirection;

    Raycasting() = default;

    void update(GLFWwindow *window, const Camera& camera, float screenWidth, float screenHeight, float fov) {
      double mouseX, mouseY;
      glfwGetCursorPos(window, &mouseX, &mouseY);

      float ndcX = (2.0f * (float)mouseX) / screenWidth - 1.0f;
      float ndcY = 1.0f - (2.0f * (float)mouseY) / screenHeight;

      glm::mat4 invView = glm::inverse(camera.getViewMatrix());
      glm::mat4 invProj = glm::inverse(glm::perspective(glm::radians(fov), screenWidth / screenHeight, 0.1f, 1000.0f));

      glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
      glm::vec4 rayEye = invProj * rayClip;
      rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

      glm::vec3 worldRayDir = glm::normalize(glm::vec3(invView*rayEye));
      glm::vec3 worldRayOrigin = camera.getPosition();

      rayOrigin = worldRayOrigin;
      rayDirection = worldRayDir;
    }
    glm::vec3 getRayOrigin() const { return rayOrigin; }
    glm::vec3 getRayDirection() const { return rayDirection; }
};


#endif
