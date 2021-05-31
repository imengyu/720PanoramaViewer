#pragma once
#include "stdafx.h"

//Shader ·â×°Àà
class CCShader
{
public:
    // the program ID
    unsigned int ID = -1;

    // constructor reads and builds the shader
    CCShader(const wchar_t* vertexPath, const wchar_t* fragmentPath);
    ~CCShader();

    // use/activate the shader
    void Use() const;

    //GetUniformLocation
    GLint GetUniformLocation(const char* name) const;
    GLint GetUniformLocation(const std::string& name) const;

    // utility uniform functions
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetBool(GLint location, bool value) const;
    void SetInt(GLint location, int value) const;
    void SetFloat(GLint location, float value) const;

    GLint viewLoc = -1;
    GLint projectionLoc = -1;
    GLint modelLoc = -1;

    bool IsSuccess() { return ID > 0; }
    bool IsNotSupport() { return errNotSupport; }
    bool IsFileMissing() { return errFileMissing; }

    void AddBindAttribLocation(std::string name) { bindAttribLocations.push_back(name); }

    bool Init();
private:

    std::wstring vertexPath;
    std::wstring fragmentPath;
    bool errNotSupport = false;
    bool errFileMissing = false;
    std::list<std::string> bindAttribLocations;
};

