#ifndef VIEW_CAMERA_H
#define VIEW_CAMERA_H
#include <cstdlib>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "../headers/time.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <functional>

const float mapSize = 3000.0f; // bumped up to match the larger terrain

class Camera {
  public:
    Camera()
      : cameraPos(0.0f, 20.0f, 3.0f),  // start high so we don't spawn inside terrain
      cameraFront(0.0f, 0.0f, -1.0f),
      cameraUp(0.0f, 1.0f, 0.0f),
      deltaTime(0.0f),
      lastFrame(0.0f),
      yaw(-90.0f),
      pitch(0.0f),
      firstMouse(true),
      lastX(400.0f),
      lastY(300.0f),
      verticalVelocity(0.0f),
      isGrounded(false),
      heightSampler(nullptr)
  {}

  glm::vec3 getPosition() const {
    return cameraPos;
  }

  void setPosition(const glm::vec3& pos) {
    cameraPos = pos;
    verticalVelocity = 0.0f;
    isGrounded = false; // let physics resolve next frame
  }

  // Hook the terrain into the camera. Pass a lambda like:
  //   camera.setHeightSampler([&](float x, float z){ return terrain.getHeightAt(x,z); });
  // Once set, the camera follows the terrain surface instead of flat ground.
  void setHeightSampler(std::function<float(float, float)> sampler) {
    heightSampler = std::move(sampler);
  }

  void update() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
  }

  void processInput(GLFWwindow* window) {
      const float cameraSpeed = 5.5f * Time::deltaTime;
      glm::vec3 flatFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
      glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, cameraUp));
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
          cameraPos += cameraSpeed * cameraFront;
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
          cameraPos -= cameraSpeed * cameraFront;
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
          cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
          cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
      if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isGrounded) {
        verticalVelocity = jumpStrength;
        isGrounded = false;
      }

      // Apply gravity
      verticalVelocity -= gravity * Time::deltaTime;
      cameraPos.y += verticalVelocity * Time::deltaTime;

      // Determine ground height at current (x, z): terrain if available, else flat 0
      float ground = heightSampler
          ? heightSampler(cameraPos.x, cameraPos.z)
          : groundLevel;

      // Snap to ground (only while inside the playable area)
      if (cameraPos.y <= ground + eyeHeight
          && std::abs(cameraPos.x) < mapSize
          && std::abs(cameraPos.z) < mapSize)
      {
        cameraPos.y = ground + eyeHeight;
        verticalVelocity = 0.0f;
        isGrounded = true;
      }
  }

  void handleMouse(double xpos, double ypos) {
    if(firstMouse) {
      lastX = static_cast<float>(xpos);
      lastY = static_cast<float>(ypos);
      firstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
  }

  glm::mat4 getViewMatrix() const {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }

private:
  static constexpr float gravity = 20.0f;
  static constexpr float jumpStrength = 7.0f;
  static constexpr float eyeHeight = 1.7f;
  static constexpr float groundLevel = 0.0f;
  glm::vec3 cameraPos;
  glm::vec3 cameraFront;
  glm::vec3 cameraUp;
  float deltaTime;
  float lastFrame;
  float yaw;
  float pitch;
  bool firstMouse;
  float lastX;
  float lastY;
  float verticalVelocity;
  bool isGrounded;
  std::function<float(float, float)> heightSampler;
};
#endif
