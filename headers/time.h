#ifndef TIME_H
#define TIME_H
#include <GLFW/glfw3.h>

namespace Time {
  inline float deltaTime = 0.0f;
  inline float lastFrame = 0.0f;
  inline float targetFrameTime = 0.0f;

  inline void update() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
  }

  inline void limitFramerate() {
    if(targetFrameTime <= 0.0f) return;
    float frameEnd = static_cast<float>(glfwGetTime());
    float elapsed = frameEnd - lastFrame;
    if(elapsed < targetFrameTime) {
      // det ska finnas bättre sätt att göra detta på
      while(static_cast<float>(glfwGetTime()) - lastFrame < targetFrameTime) {}
    }
  }
}

#endif
