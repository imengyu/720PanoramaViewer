#include "CCShader.h"
#include "CStringHlp.h"
#include "720Core.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

CCShader::CCShader(const wchar_t* vertexPath, const wchar_t* fragmentPath)
{
    this->vertexPath = vertexPath;
    this->fragmentPath = fragmentPath;
}
CCShader::~CCShader()
{
    glDeleteProgram(ID);
}

bool CCShader::Init() {
    // 1. retrieve the vertex/fragment source code from filePath
    std::wstring vertexCode;
    std::wstring fragmentCode;

    std::wifstream vShaderFile;
    std::wifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::wifstream::failbit | std::wifstream::badbit);
    fShaderFile.exceptions(std::wifstream::failbit | std::wifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::wstringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::wifstream::failure e)
    {
        LOGEF(L"[CCShader] Read shader file failed! %s (%d)", e.code().message().c_str(), e.code().value());
        errFileMissing = true;
        return false;
    }

    std::string vShaderCode = CStringHlp::UnicodeToAnsi(vertexCode);
    std::string fShaderCode = CStringHlp::UnicodeToAnsi(fragmentCode);

    const char* vfShaderCode = vShaderCode.c_str();
    const char* ffShaderCode = fShaderCode.c_str();

    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vfShaderCode, nullptr);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        LOGEF(L"[CCShader] Compile vertex shader file failed! \n%hs", infoLog);
        errNotSupport = CStringHlp::StrContainsA(infoLog, "not supported", nullptr);
        return false;
    };

    // similiar for Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &ffShaderCode, nullptr);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        LOGEF(L"[CCShader] Compile fragment shader file failed! \n%hs", infoLog);
        errNotSupport = CStringHlp::StrContainsA(infoLog, "not supported", nullptr);
        return false;
    };

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    //BindAttribLocations
    int i = 0;
    std::list<std::string>::iterator iter = bindAttribLocations.begin();
    for (; iter != bindAttribLocations.end(); iter++, i++)
        glBindAttribLocation(ID, i, (*iter).c_str());

    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        LOGEF(L"[CCShader] Link shader program failed! \n%hs", infoLog);
        return false;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    viewLoc = GetUniformLocation("view");
    projectionLoc = GetUniformLocation("projection");
    modelLoc = GetUniformLocation("model");

    return true;
}
void CCShader::Use() const
{
    glUseProgram(ID);
}

GLint CCShader::GetUniformLocation(const char* name) const
{
    return glGetUniformLocation(ID, name);
}
GLint CCShader::GetUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(ID, name.c_str());
}
void CCShader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void CCShader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void CCShader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void CCShader::SetBool(GLint location, bool value) const
{
    glUniform1i(location, (int)value);
}
void CCShader::SetInt(GLint location, int value) const
{
    glUniform1i(location, value);
}
void CCShader::SetFloat(GLint location, float value) const
{
    glUniform1f(location, value);
}
