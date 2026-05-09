#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets/terrain.h"
#include "shader/shader.h"
#include "assets/view_camera.h"
#include "assets/model.h"
#include "assets/muzzle_flash.h"
#include "shader/raycasting.h"
#include "headers/audio.h"
#include "headers/time.h"
#include "assets/skybox.h"
//#include "headers/text_renderer.h"

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
  Raycasting raycaster;
  Audio::init();
  MuzzleFlash muzzleFlash("resources/muzzle_flash.png");
  glfwSetWindowUserPointer(window, &camera);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Shader jaegerShader("assets/6.3.coordinate_system.vs", "assets/6.3.coordinate_system.fs");
  Terrain terrain(6000.0f, 500, 0.0f,
                  "resources/ground_texture.jpg",
                  "resources/rock_texture.jpg",
                  "resources/snow_texture.jpg");

  camera.setHeightSampler([&terrain](float x, float z) {
      return terrain.getHeightAt(x, z);
  });
  camera.setPosition(glm::vec3(0.0f, terrain.getHeightAt(0, 0) + 5.0f, 0.0f));

  Model pistol("resources/3d-sculptures/9mm_pistol/nv_9mm.obj", "resources/3d-sculptures/9mm_pistol/9mm.png");
  Model bottle("resources/3d-sculptures/columbia_whiskey/Whiskey_Bottle.obj", "resources/3d-sculptures/columbia_whiskey/WhiskeyBottle_DIFF.png");

  std::vector<std::string> skyboxFaces = {
    "resources/skybox/miramar_rt.tga",
    "resources/skybox/miramar_lf.tga",
    "resources/skybox/miramar_up.tga",
    "resources/skybox/miramar_dn.tga",
    "resources/skybox/miramar_bk.tga",
    "resources/skybox/miramar_ft.tga"
  };

  Shader skyboxShader("assets/skybox.vs", "assets/skybox.fs");
  Skybox skybox(skyboxFaces);
  skyboxShader.use();
  skyboxShader.setInt("skybox", 0);

  jaegerShader.use();
  jaegerShader.setInt("texture1", 0);

  jaegerShader.setVec3("uSunDir", glm::normalize(glm::vec3(-0.3f, 0.9f, -0.2f)));
  jaegerShader.setVec3("uSunColor",      glm::vec3(0.85f, 0.85f, 0.92f));
  jaegerShader.setVec3("uAmbientSky",    glm::vec3(0.55f, 0.60f, 0.70f));
  jaegerShader.setVec3("uAmbientGround", glm::vec3(0.20f, 0.20f, 0.22f));
  jaegerShader.setVec3("uFogColor",      glm::vec3(0.62f, 0.65f, 0.70f));
  jaegerShader.setFloat("uFogStart",     25.0f);
  jaegerShader.setFloat("uFogEnd",       110.0f);

  //TextRenderer textRenderer("resources/fonts/DejaVuSans.ttf");

  Time::targetFrameTime = 1.0f / 60.0f;

  float muzzleFlashTimer = 0.0f;
  bool prevMousePressed = false;
  const float fireCooldown = 0.15;
  float cooldownRemaining = 0.0f;
  float recoilAmount = 0.0f;
  const float recoilRecoverySpeed = 5.0f;

  while(!glfwWindowShouldClose(window)) {
    Time::update();
    processInput(window);
    camera.processInput(window);
    raycaster.update(window, camera, SRC_WIDTH, SRC_HEIGHT, 45.0f);
    if (cooldownRemaining > 0.0f) cooldownRemaining -= Time::deltaTime;
    if (muzzleFlashTimer > 0.0f) muzzleFlashTimer -= Time::deltaTime;
    if (recoilAmount > 0.0f) {
      recoilAmount -= Time::deltaTime * recoilRecoverySpeed;
      if (recoilAmount < 0.0f) recoilAmount = 0.0f;
    }

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (mousePressed && !prevMousePressed && cooldownRemaining <= 0.0f) {
      Audio::playOneShot("resources/sounds/gunshot.wav");
      muzzleFlashTimer = 0.05f;
      cooldownRemaining = fireCooldown;
      recoilAmount = 1.0f;

      glm::vec3 origin = raycaster.getRayOrigin();
      glm::vec3 dir = raycaster.getRayDirection();
      std::cout << "Ray: " << origin.x << " " << origin.y << " " << origin.z << std::endl;
    }
    prevMousePressed = mousePressed;

    // ---------- CLEAR ----------
    glClearColor(0.62f, 0.65f, 0.70f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---------- SKYBOX (drawn first, behind everything) ----------
    skyboxShader.use();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.getViewMatrix()));
    skyboxShader.setMat4("view", skyboxView);
    glm::mat4 skyboxProjection = glm::perspective(glm::radians(45.0f),
                                                  (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                                  0.1f, 1000.0f);
    skyboxShader.setMat4("projection", skyboxProjection);
    skybox.draw();

    // ---------- WORLD SHADER SETUP ----------
    jaegerShader.use();
    jaegerShader.setVec3("uCameraPos", camera.getPosition());
    // NOTE: bumped far plane to 500 so big terrain doesn't get clipped
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                            0.1f, 500.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    jaegerShader.setMat4("projection", projection);
    jaegerShader.setMat4("view", view);
    jaegerShader.setMat4("model", model);

    // ---------- TERRAIN ----------
    jaegerShader.setBool("uTerrain", true);
    jaegerShader.setInt("grassTex", 0);
    jaegerShader.setInt("rockTex", 1);
    jaegerShader.setInt("snowTex", 2);   // FIX: was 1
    terrain.draw();
    jaegerShader.setBool("uTerrain", false);
    jaegerShader.setInt("texture1", 0); // models use unit 0

    // ---------- VIEWMODEL (pistol + bottle): identity view ----------
    glClear(GL_DEPTH_BUFFER_BIT);  // so viewmodel never clips into terrain
    jaegerShader.setMat4("view", glm::mat4(1.0f));
    jaegerShader.setBool("uViewmodel", true);

    glm::mat4 pistolModel = glm::mat4(1.0f);
    pistolModel = glm::translate(pistolModel, glm::vec3(0.25f, -0.3f, -1.3f));
    pistolModel = glm::translate(pistolModel, glm::vec3(0.0f, 0.0f, recoilAmount * 0.15f));
    pistolModel = glm::rotate(pistolModel, glm::radians(recoilAmount * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    pistolModel = glm::scale(pistolModel, glm::vec3(0.6f));
    pistolModel = glm::rotate(pistolModel, glm::radians(-25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    pistolModel = glm::rotate(pistolModel, glm::radians(10.0f),  glm::vec3(1.0f, 0.0f, 0.0f));
    pistolModel = glm::rotate(pistolModel, glm::radians(95.0f),  glm::vec3(0.0f, 1.0f, 0.0f));
    pistolModel = glm::translate(pistolModel, glm::vec3(-8.0f, 0.4f, 4.4f));

    glm::mat4 bottleModel = glm::mat4(1.0f);
    bottleModel = glm::translate(bottleModel, glm::vec3(-0.4f, -0.33f, -1.3f));
    bottleModel = glm::scale(bottleModel, glm::vec3(0.008f));
    bottleModel = glm::rotate(bottleModel, glm::radians(5.0f),   glm::vec3(1.0f, 0.0f, 1.0f));
    bottleModel = glm::rotate(bottleModel, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bottleModel = glm::rotate(bottleModel, glm::radians(45.0f),  glm::vec3(0.0f, 1.0f, 0.0f));
    bottleModel = glm::translate(bottleModel, glm::vec3(0.0f, -30.87f, 0.0f));

    if (muzzleFlashTimer > 0.0f) {
      glm::mat4 flashModel = glm::mat4(1.0f);
      flashModel = glm::translate(flashModel, glm::vec3(0.15f, -0.07f, -1.5f));
      flashModel = glm::scale(flashModel, glm::vec3(0.3f));
      jaegerShader.setMat4("model", flashModel);
      muzzleFlash.draw();
    }

    jaegerShader.setMat4("model", pistolModel);
    pistol.draw();

    glDisable(GL_CULL_FACE);
    jaegerShader.setMat4("model", bottleModel);
    bottle.draw();

    jaegerShader.setBool("uViewmodel", false);

    // ---------- HUD ----------
    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //textRenderer.RenderText("JÄGER", 25.0f, 560.0f, 0.8f, glm::vec3(1.0f, 0.8f, 0.2f));
    //glEnable(GL_DEPTH_TEST);

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
