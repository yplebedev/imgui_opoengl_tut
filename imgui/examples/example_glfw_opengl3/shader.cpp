#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "glad/glad.h"

class Shader {
    public:
        unsigned int ID;
        Shader(const char* vertexPath, const char* fragmentPath) {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                // open files
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                // read file’s buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();
                // close file handlers
                vShaderFile.close();
                fShaderFile.close();
                // convert stream into string
                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch(std::ifstream::failure& _)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            }
            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();

            // 2. compile shaders
            unsigned int vertex, fragment;
            int successV, successF, successL;
            char infoLog[512];
            // vertex Shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, nullptr);
            glCompileShader(vertex);
            // print compile errors if any
            glGetShaderiv(vertex, GL_COMPILE_STATUS, &successV);
            if(!successV)
            {
                glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" <<
                infoLog << std::endl;
            }
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, nullptr);
            glCompileShader(fragment);
            glGetShaderiv(fragment, GL_COMPILE_STATUS, &successF);
            if(!successF)
            {
                glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
                std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" <<
                infoLog << std::endl;
            }
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);
            // print linking errors if any
            glGetProgramiv(ID, GL_LINK_STATUS, &successL);
            if(!successL)
            {
                glGetProgramInfoLog(ID, 512, nullptr, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" <<
                infoLog << std::endl;
            }
            // delete shaders; they’re linked into our program and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        void use() const {
            glUseProgram(ID);
        };
        void setBool(const std::string& name, bool value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
        };
        void setInt(const std::string& name, int value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        };
        void setFloat(const std::string& name, float value) const {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        };
};
