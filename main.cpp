#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets/ground.h"
#include "shader/shader.h"
#include "assets/view_camera.h"
#include "assets/model.h"
#include "assets/muzzle_flash.h"
#include "headers/audio.h"
#include "headers/time.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

const int SRC_WIDTH = 800;
const int SRC_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
  if(cam) cam->handleMouse(xpos, ypos);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Jäger", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "ERROR::GLAD::LOADER::INITIALIZATION" << std::endl;
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  stbi_set_flip_vertically_on_load(true);

  Camera camera;
  Audio::init();
  MuzzleFlash muzzleFlash("resources/muzzle_flash.png");
  glfwSetWindowUserPointer(window, &camera);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Shader jaegerShader("assets/6.3.coordinate_system.vs", "assets/6.3.coordinate_system.fs");
  Ground mapGround(100.0f, 50, 0.0f, "resources/ground_texture.jpg");
  Model pistol("resources/3d-sculptures/9mm_pistol/nv_9mm.obj", "resources/3d-sculptures/9mm_pistol/9mm.png");

  jaegerShader.use();
  jaegerShader.setInt("texture1", 0);

  // fps deklaration
  Time::targetFrameTime = 1.0f / 60.0f;

  float muzzleFlashTimer = 0.0f;
  bool prevMousePressed = false;
  const float fireCooldown = 0.15;
  float cooldownRemaining = 0.0f;

  while(!glfwWindowShouldClose(window)) {
    Time::update();
    processInput(window);
    camera.processInput(window);
    if (cooldownRemaining > 0.0f) cooldownRemaining -= Time::deltaTime;
    if (muzzleFlashTimer > 0.0f) muzzleFlashTimer -= Time::deltaTime;

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (mousePressed && !prevMousePressed && cooldownRemaining <= 0.0f) {
      Audio::playOneShot("resources/sounds/gunshot.wav");
      muzzleFlashTimer = 0.05f;
      cooldownRemaining = fireCooldown;
    }
    prevMousePressed = mousePressed;
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    jaegerShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.getViewMatrix();

    glm::mat4 model = glm::mat4(1.0f);

    jaegerShader.setMat4("projection", projection);
    jaegerShader.setMat4("view", view);

    // Draw ground
    jaegerShader.setMat4("model", model);
    mapGround.draw();

    // Draw pistol
    jaegerShader.setMat4("view", glm::mat4(1.0f));
    glm::mat4 pistolModel = glm::mat4(1.0f);
    pistolModel = glm::translate(pistolModel, glm::vec3(0.3f, -0.3f, -1.3f));
    pistolModel = glm::scale(pistolModel, glm::vec3(0.6f));
    pistolModel = glm::rotate(pistolModel, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    pistolModel = glm::rotate(pistolModel, glm::radians(90.0f), glm::vec3(0.0f, 0.4f, 0.0f));
    pistolModel = glm::translate(pistolModel, glm::vec3(-8.0f, 0.4f, 4.4f));

    glClear(GL_DEPTH_BUFFER_BIT);

    if (muzzleFlashTimer > 0.0f) {
      glm::mat4 flashModel = glm::mat4(1.0f);
      //                                                 x,     y,     z
      flashModel = glm::translate(flashModel, glm::vec3(0.15f, -0.15f, -1.5f));
      flashModel = glm::scale(flashModel, glm::vec3(0.3f));
      jaegerShader.setMat4("model", flashModel);
      muzzleFlash.draw();
    }

    jaegerShader.setMat4("model", pistolModel);
    pistol.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
    Time::limitFramerate();
  }

  Audio::shutdown();
  glfwTerminate();

  return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}
