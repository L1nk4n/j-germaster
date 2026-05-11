
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

class LightProperties {
  public:
    glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    float constant = 1.0f;
    float linear = 0.045f;
    float quadratic = 0.0075f;
};

class PointLight {
  public:
    PointLight(glm::vec3 pos) {
      position = pos;
    }
    PointLight(glm::vec3 pos, LightProperties props) {
      position = pos;
      lightProperties = props;
    }

    LightProperties lightProps() {
      return lightProperties;
    }
    glm::vec3 getPos() {
      return position;
    }
  private:
    glm::vec3 position;
    LightProperties lightProperties;
};

class DirLight {
  public:
    DirLight(glm::vec3 dir) {
      direction = dir;
    }
    DirLight(glm::vec3 dir, LightProperties props) {
      direction = dir;
      lightProperties = props;
    }

    LightProperties lightProps() {
      return lightProperties;
    }
    glm::vec3 getDir() {
      return direction;
    }
  private:
    glm::vec3 direction;
    LightProperties lightProperties;
};

class FlashLight {
  public:
    FlashLight(float innerCutOff, float outerCutOff, LightProperties props = LightProperties {}) {
      innerCutOff_ = innerCutOff;
      outerCutOff_ = outerCutOff;
      lightProperties = props;
    }

    float getInnerCutOff() {
      return innerCutOff_;
    }
    float getOuterCutOff() {
      return outerCutOff_;
    }

    LightProperties lightProps() {
      return lightProperties;
    }

  private:
    float innerCutOff_;
    float outerCutOff_;
    LightProperties lightProperties;
};
