#include "pch.h"

#ifdef utjSupportOpenGL
#include "Foundation.h"
#include "GraphicsDevice.h"

#ifndef utjWindows
    #define GLEW_STATIC
#endif
#include <GL/glew.h>

#if defined(utjWindows)
#pragma comment(lib, "opengl32.lib")
//#pragma comment(lib, "glew32s.lib")
#endif

namespace utj {
class GraphicsDeviceOpenGL : public GraphicsDevice
{
public:
    GraphicsDeviceOpenGL();
    ~GraphicsDeviceOpenGL();
    void* getDevicePtr() override;
    GfxDeviceType getDeviceType() override;
    void sync() override;
    bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) override;
    bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) override;
};


GraphicsDevice* CreateGraphicsDeviceOpenGL()
{
    return new GraphicsDeviceOpenGL();
}


void* GraphicsDeviceOpenGL::getDevicePtr() { return nullptr; }
GfxDeviceType GraphicsDeviceOpenGL::getDeviceType() { return GfxDeviceType_OpenGL; }

GraphicsDeviceOpenGL::GraphicsDeviceOpenGL()
{
//    glewInit();
}

GraphicsDeviceOpenGL::~GraphicsDeviceOpenGL()
{
}


static void GetInternalFormatOpenGL(PixelFormat format, GLenum &o_fmt, GLenum &o_type)
{
    switch (format)
    {
    case PixelFormat_RGBAu8:    o_fmt = GL_RGBA; o_type = GL_UNSIGNED_BYTE; return;

    case PixelFormat_RGBAf16:  o_fmt = GL_RGBA; o_type = GL_HALF_FLOAT; return;
    case PixelFormat_RGf16:    o_fmt = GL_RG; o_type = GL_HALF_FLOAT; return;
    case PixelFormat_Rf16:     o_fmt = GL_RED; o_type = GL_HALF_FLOAT; return;

    case PixelFormat_RGBAf32: o_fmt = GL_RGBA; o_type = GL_FLOAT; return;
    case PixelFormat_RGf32:   o_fmt = GL_RG; o_type = GL_FLOAT; return;
    case PixelFormat_Rf32:    o_fmt = GL_RED; o_type = GL_FLOAT; return;

    case PixelFormat_RGBAi32:   o_fmt = GL_RGBA_INTEGER; o_type = GL_INT; return;
    case PixelFormat_RGi32:     o_fmt = GL_RG_INTEGER; o_type = GL_INT; return;
    case PixelFormat_Ri32:      o_fmt = GL_RED_INTEGER; o_type = GL_INT; return;
    default: break;
    }
}

void GraphicsDeviceOpenGL::sync()
{
    glFinish();
}

bool GraphicsDeviceOpenGL::readTexture(void *o_buf, size_t, void *tex, int, int, PixelFormat format)
{
    GLenum internal_format = 0;
    GLenum internal_type = 0;
    GetInternalFormatOpenGL(format, internal_format, internal_type);

    //// glGetTextureImage() is available only OpenGL 4.5 or later...
    // glGetTextureImage((GLuint)(size_t)tex, 0, internal_format, internal_type, bufsize, o_buf);

    sync();
    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)tex);
    glGetTexImage(GL_TEXTURE_2D, 0, internal_format, internal_type, o_buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool GraphicsDeviceOpenGL::writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t)
{
    GLenum internal_format = 0;
    GLenum internal_type = 0;
    GetInternalFormatOpenGL(format, internal_format, internal_type);

    //// glTextureSubImage2D() is available only OpenGL 4.5 or later...
    // glTextureSubImage2D((GLuint)(size_t)o_tex, 0, 0, 0, width, height, internal_format, internal_type, buf);

    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)o_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, internal_format, internal_type, buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

} // namespace utj
#endif // utjSupportOpenGL
