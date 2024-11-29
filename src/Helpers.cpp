#include "Helpers.h"

#include <exception>
#include <unordered_set>

#include <hyprland/src/Compositor.hpp>
#include <hyprutils/string/String.hpp>

void ShaderHolder::Init() {
    g_pHyprRenderer->makeEGLCurrent();

    GLuint prog                    = CreateProgram(TEXVERTSRC, TEXFRAGSRCRGBA_CHROMA);
    RGBA.cshader.program           = prog;
    RGBA.cshader.proj              = glGetUniformLocation(prog, "proj");
    RGBA.cshader.tex               = glGetUniformLocation(prog, "tex");
    RGBA.cshader.alpha             = glGetUniformLocation(prog, "alpha");
    RGBA.cshader.texAttrib         = glGetAttribLocation(prog, "texcoord");
    RGBA.cshader.posAttrib         = glGetAttribLocation(prog, "pos");
    RGBA.cshader.discardOpaque     = glGetUniformLocation(prog, "discardOpaque");
    RGBA.cshader.discardAlpha      = glGetUniformLocation(prog, "discardAlpha");
    RGBA.cshader.discardAlphaValue = glGetUniformLocation(prog, "discardAlphaValue");
    RGBA.cshader.topLeft           = glGetUniformLocation(prog, "topLeft");
    RGBA.cshader.fullSize          = glGetUniformLocation(prog, "fullSize");
    RGBA.cshader.radius            = glGetUniformLocation(prog, "radius");
    RGBA.cshader.applyTint         = glGetUniformLocation(prog, "applyTint");
    RGBA.cshader.tint              = glGetUniformLocation(prog, "tint");
    RGBA.bkg                       = glGetUniformLocation(prog, "bkg");
    RGBA.chromaAlpha               = glGetUniformLocation(prog, "chromaAlpha");
    RGBA.similarity                = glGetUniformLocation(prog, "similarity");

    prog                          = CreateProgram(TEXVERTSRC, TEXFRAGSRCEXT_CHROMA);
    EXT.cshader.program           = prog;
    EXT.cshader.tex               = glGetUniformLocation(prog, "tex");
    EXT.cshader.proj              = glGetUniformLocation(prog, "proj");
    EXT.cshader.alpha             = glGetUniformLocation(prog, "alpha");
    EXT.cshader.posAttrib         = glGetAttribLocation(prog, "pos");
    EXT.cshader.texAttrib         = glGetAttribLocation(prog, "texcoord");
    EXT.cshader.discardOpaque     = glGetUniformLocation(prog, "discardOpaque");
    EXT.cshader.discardAlpha      = glGetUniformLocation(prog, "discardAlpha");
    EXT.cshader.discardAlphaValue = glGetUniformLocation(prog, "discardAlphaValue");
    EXT.cshader.topLeft           = glGetUniformLocation(prog, "topLeft");
    EXT.cshader.fullSize          = glGetUniformLocation(prog, "fullSize");
    EXT.cshader.radius            = glGetUniformLocation(prog, "radius");
    EXT.cshader.applyTint         = glGetUniformLocation(prog, "applyTint");
    EXT.cshader.tint              = glGetUniformLocation(prog, "tint");
    EXT.bkg                       = glGetUniformLocation(prog, "bkg");
    EXT.chromaAlpha               = glGetUniformLocation(prog, "chromaAlpha");
    EXT.similarity                = glGetUniformLocation(prog, "similarity");

    g_pHyprRenderer->unsetEGL();
}

void ShaderHolder::Destroy() {
    g_pHyprRenderer->makeEGLCurrent();

    RGBA.cshader.destroy();
    EXT.cshader.destroy();

    g_pHyprRenderer->unsetEGL();
}

GLuint ShaderHolder::CompileShader(const GLuint& type, std::string src) {
    auto shader = glCreateShader(type);

    auto shaderSource = src.c_str();

    glShaderSource(shader, 1, (const GLchar**)&shaderSource, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        Debug::log(ERR, "Error compiling shader: %s", infoLog);
        throw std::runtime_error(std::string("Error compiling shader: ") + infoLog);
    }

    return shader;
}

GLuint ShaderHolder::CreateProgram(const std::string& vert, const std::string& frag) {
    auto vertCompiled = CompileShader(GL_VERTEX_SHADER, vert);
    auto fragCompiled = CompileShader(GL_FRAGMENT_SHADER, frag);

    auto prog = glCreateProgram();
    glAttachShader(prog, vertCompiled);
    glAttachShader(prog, fragCompiled);
    glLinkProgram(prog);

    glDetachShader(prog, vertCompiled);
    glDetachShader(prog, fragCompiled);
    glDeleteShader(vertCompiled);
    glDeleteShader(fragCompiled);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char infoLog[512];
        glGetShaderInfoLog(prog, 512, NULL, infoLog);
        Debug::log(ERR, "Error linking shader: %s", infoLog);
        throw std::runtime_error(std::string("Error linking shader: ") + infoLog);
    }

    return prog;
}
