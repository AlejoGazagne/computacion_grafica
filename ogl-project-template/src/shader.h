#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID;
    bool in_use_;

    // Constructor lee y construye el shader
    Shader(const char* vertexPath, const char* fragmentPath) : in_use_(false) {
        // 1. Recuperar el código fuente de los archivos
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        // Asegurar que los objetos ifstream pueden lanzar excepciones
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            // Abrir archivos
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            // Leer el contenido de los buffers de los archivos
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // Cerrar los archivos
            vShaderFile.close();
            fShaderFile.close();

            // Convertir stream en string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR: No se pudieron leer los archivos de shader: " << e.what() << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compilar shaders
        unsigned int vertex, fragment;
        
        // Vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // Fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Eliminar los shaders ya que están linkeados en el programa
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Activar el shader
    void use() {
        if (!in_use_) {
            glUseProgram(ID);
            in_use_ = true;
        }
    }

    // Desactivar el shader
    void disable() {
        if (in_use_) {
            glUseProgram(0);
            in_use_ = false;
        }
    }

    // Verificar si el shader está en uso
    bool isInUse() const {
        return in_use_;
    }

    // Función interna para asegurar que el shader está activo
    void ensureActive() {
        if (!in_use_) {
            glUseProgram(ID);
            in_use_ = true;
        }
    }

    // Funciones de utilidad para setear uniforms (activación automática)
    void setBool(const std::string &name, bool value) {
        ensureActive();
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    
    void setInt(const std::string &name, int value) {
        ensureActive();
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setFloat(const std::string &name, float value) {
        ensureActive();
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, float x, float y) {
        ensureActive();
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    
    void setVec3(const std::string &name, float x, float y, float z) {
        ensureActive();
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    
    void setVec4(const std::string &name, float x, float y, float z, float w) {
        ensureActive();
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

private:
    // Función para verificar errores de compilación/linking
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR: Compilación de SHADER falló: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR: Linking de PROGRAMA falló: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif