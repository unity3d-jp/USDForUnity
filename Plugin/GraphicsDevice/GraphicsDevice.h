#ifndef utj_GraphicsDevice_h
#define utj_GraphicsDevice_h

namespace utj {

enum GfxDeviceType
{
    GfxDeviceType_Unknown,
    GfxDeviceType_D3D9,
    GfxDeviceType_D3D11,
    GfxDeviceType_D3D12,
    GfxDeviceType_OpenGL,
    GfxDeviceType_Vulkan,
};


class GraphicsDevice
{
public:
    virtual ~GraphicsDevice() {}
    virtual void* getDevicePtr() = 0;
    virtual GfxDeviceType getDeviceType() = 0;
    virtual void sync() = 0;
    virtual bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) = 0;
    virtual bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) = 0;
};
utjCLinkage utjExport GraphicsDevice* GetGraphicsDevice();

} // namespace utj
#endif // utj_GraphicsDevice_h
