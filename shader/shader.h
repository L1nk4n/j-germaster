#ifndef SHADER_H
#define SHADER_H

#include "../assets/light.h"
#include "../assets/light_source.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>
#include <vector>

class Shader {
  public:
    unsigned int ID;
    Shader(
        std::string vsPath, 
        std::string fsPath, 
        std::vector<std::string> vsOptions, 
        std::vector<std::string> fsOptions
      ) {
      vertexPath = vsPath;
      fragmentPath = fsPath;
      vertexOptions = vsOptions;
      fragmentOptions = fsOptions;

      auto [vertex, fragment] = compileShaders(vertexPath, fragmentPath);

      ID = glCreateProgram();
      glAttachShader(ID, vertex);
      glAttachShader(ID, fragment);
      glLinkProgram(ID);
      checkCompileErrors(ID, "PROGRAM");
      glDeleteShader(vertex);
      glDeleteShader(fragment);

      updateLights();
    }

    void recompileShaders() {
      auto [vertex, fragment] = compileShaders(vertexPath, fragmentPath);


      glDeleteProgram(ID);
      ID = glCreateProgram();
      glAttachShader(ID, vertex);
      glAttachShader(ID, fragment);
      glLinkProgram(ID);
      checkCompileErrors(ID, "PROGRAM");
      glDeleteShader(vertex);
      glDeleteShader(fragment);

      updateLights();
    }

    void addDirLight(glm::vec3 direction, LightProperties props = LightProperties {}) {
      dirLights.push_back(DirLight(direction, props));
      use();
      setLightProp(fmt::format("dirLights[{}]", dirLights.size()-1), dirLights[dirLights.size()-1].lightProps());
      setVec3(fmt::format("dirLights[{}].direction",  dirLights.size()-1), dirLights[dirLights.size()-1].getDir());
    }

    void addPointLight(glm::vec3 position, LightProperties props = LightProperties {}) {
      pointLights.push_back(PointLight(position, props));
      use();
      setLightProp(fmt::format("pointLights[{}]", pointLights.size()-1), pointLights[pointLights.size()-1].lightProps());
      setVec3(fmt::format("pointLights[{}].position",  pointLights.size()-1), pointLights[pointLights.size()-1].getPos());
    }

    void addFlashLight(float innerCutOff, float outerCutOff, LightProperties props = LightProperties {}) {
      flashLights.push_back(FlashLight(innerCutOff, outerCutOff, props));
      use();
      setLightProp(fmt::format("flashLights[{}]", flashLights.size()-1), flashLights[flashLights.size()-1].lightProps());
      setFloat(fmt::format("flashLights[{}].innerCutOff",  flashLights.size()-1), flashLights[flashLights.size()-1].getInnerCutOff());
      setFloat(fmt::format("flashLights[{}].outerCutOff",  flashLights.size()-1), flashLights[flashLights.size()-1].getOuterCutOff());
    }

    void updateLights() {
      use();
      for (int i = 0; i < dirLights.size(); i++) {
        setLightProp(fmt::format("dirLights[{}]", i), dirLights[i].lightProps());
        setVec3(fmt::format("dirLights[{}].direction", i), dirLights[i].getDir());
      }
      for (int i = 0; i < pointLights.size(); i++) {
        setLightProp(fmt::format("pointLights[{}]", i), pointLights[i].lightProps());
        setVec3(fmt::format("pointLights[{}].position", i), pointLights[i].getPos());
      }
      for (int i = 0; i < flashLights.size(); i++) {
        setLightProp(fmt::format("flashLights[{}]", i), flashLights[i].lightProps());
        setFloat(fmt::format("flashLights[{}].innerCutOff", i),flashLights[i].getInnerCutOff());
        setFloat(fmt::format("flashLights[{}].outerCutOff", i),flashLights[i].getOuterCutOff());
      }
    }

    void use() {
      glUseProgram(ID);
    }

    void setVec3(const std::string &name, glm::vec3 vec) const {
      glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
      glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void setBool(const std::string &name, bool value) const {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
      glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
      glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setLightProp(const std::string &name, LightProperties props) {
      setVec3(name+".ambient", props.ambient);
      setVec3(name+".diffuse", props.diffuse);
      setVec3(name+".specular", props.specular);
      setFloat(name+".constant", props.constant);
      setFloat(name+".linear", props.linear);
      setFloat(name+".quadratic", props.quadratic);
    }
  private:
    std::vector<DirLight> dirLights;
    std::vector<PointLight> pointLights;
    std::vector<FlashLight> flashLights;

    std::string vertexPath;
    std::string fragmentPath;

    std::vector<std::string> vertexOptions;
    std::vector<std::string> fragmentOptions;
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    std::tuple<int, int> compileShaders(std::string vertPath, std::string fragPath) {
      std::string vertexCode;
      std::string fragmentCode;
      std::ifstream vShaderFile;
      std::ifstream fShaderFile;

      vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
      fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
      try {
        vShaderFile.open(vertPath);
        fShaderFile.open(fragPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
      }
      catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
      }
      std::string vShaderCode = vertexCode.c_str();
      std::string fShaderCode = fragmentCode.c_str();

      if (dirLights.size() != 0)
        fShaderCode.insert(18, fmt::format("#define DIRLIGHT {}\n", dirLights.size()));
      if (pointLights.size() != 0)
        fShaderCode.insert(18, fmt::format("#define POINTLIGHT {}\n", pointLights.size()));
      if (flashLights.size() != 0)
        fShaderCode.insert(18, fmt::format("#define FLASHLIGHT {}\n", flashLights.size()));

      for (std::string opt : vertexOptions)
        vShaderCode.insert(18, std::string("#define ") + opt + "\n");
      for (std::string opt : fragmentOptions)
        fShaderCode.insert(18, std::string("#define ") + opt + "\n");

      const char* vCode = vShaderCode.c_str();
      const char* fCode = fShaderCode.c_str();

      unsigned int vertex, fragment;

      vertex = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex, 1, &vCode, NULL);
      glCompileShader(vertex);
      checkCompileErrors(vertex, "VERTEX");

      fragment = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment, 1, &fCode, NULL);
      glCompileShader(fragment);
      checkCompileErrors(fragment, "FRAGMENT");

      return std::tuple(vertex, fragment);
    }
};
#endif

