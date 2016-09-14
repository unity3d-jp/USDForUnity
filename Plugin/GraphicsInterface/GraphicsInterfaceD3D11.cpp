#include "pch.h"
#include "giInternal.h"

#ifdef giSupportD3D11
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace gi {

class GraphicsInterfaceD3D11 : public GraphicsInterface
{
public:
    GraphicsInterfaceD3D11(void *device);
    ~GraphicsInterfaceD3D11() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void   releaseTexture2D(void *tex) override;
    Result readTexture2D(void *dst, size_t read_size, void *src_tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void   releaseBuffer(void *buf) override;
    Result readBuffer(void *dst, void *src_buf, size_t read_size, BufferType type) override;
    Result writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
    enum class StagingFlag {
        Upload,
        Readback,
    };
    ComPtr<ID3D11Texture2D> createStagingTexture(int width, int height, TextureFormat format, StagingFlag flag);
    ComPtr<ID3D11Buffer> createStagingBuffer(size_t size, StagingFlag flag);

private:
    ComPtr<ID3D11Device> m_device = nullptr;
    ComPtr<ID3D11DeviceContext> m_context = nullptr;
    ComPtr<ID3D11Query> m_query_event = nullptr;
};


GraphicsInterface* CreateGraphicsInterfaceD3D11(void *device)
{
    if (!device) { return nullptr; }
    return new GraphicsInterfaceD3D11(device);
}

GraphicsInterfaceD3D11::GraphicsInterfaceD3D11(void *device)
    : m_device((ID3D11Device*)device)
{
    if (m_device != nullptr)
    {
        m_device->GetImmediateContext(&m_context);

        D3D11_QUERY_DESC qdesc = {D3D11_QUERY_EVENT , 0};
        m_device->CreateQuery(&qdesc, &m_query_event);
    }
}

GraphicsInterfaceD3D11::~GraphicsInterfaceD3D11()
{
}

void GraphicsInterfaceD3D11::release()
{
    delete this;
}

void* GraphicsInterfaceD3D11::getDevicePtr() { return m_device.Get(); }
DeviceType GraphicsInterfaceD3D11::getDeviceType() { return DeviceType::D3D11; }

void GraphicsInterfaceD3D11::sync()
{
    m_context->End(m_query_event.Get());
    while (m_context->GetData(m_query_event.Get(), nullptr, 0, 0) == S_FALSE) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}


ComPtr<ID3D11Texture2D> GraphicsInterfaceD3D11::createStagingTexture(int width, int height, TextureFormat format, StagingFlag flag)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = (UINT)width;
    desc.Height = (UINT)height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = GetDXGIFormat(format);
    desc.SampleDesc = { 1, 0 };
    desc.Usage = D3D11_USAGE_STAGING;
    if (flag == StagingFlag::Upload) {
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    if (flag == StagingFlag::Readback) {
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }

    auto ret = ComPtr<ID3D11Texture2D>();
    auto hr = m_device->CreateTexture2D(&desc, nullptr, &ret);
    return ret;
}

ComPtr<ID3D11Buffer> GraphicsInterfaceD3D11::createStagingBuffer(size_t size, StagingFlag flag)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)size;
    desc.Usage = D3D11_USAGE_STAGING;
    if (flag == StagingFlag::Upload) {
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    if (flag == StagingFlag::Readback) {
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }

    auto ret = ComPtr<ID3D11Buffer>();
    auto hr = m_device->CreateBuffer(&desc, nullptr, &ret);
    return ret;
}

Result GraphicsInterfaceD3D11::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    size_t texel_size = GetTexelSize(format);
    DXGI_FORMAT internal_format = GetDXGIFormat(format);

    D3D11_TEXTURE2D_DESC desc = {
        (UINT)width, (UINT)height, 1, 1, internal_format, { 1, 0 },
        D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0
    };
    if (flags & ResourceFlags::CPU_Write) {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }
    if (flags & ResourceFlags::CPU_Read) {
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
    }

    D3D11_SUBRESOURCE_DATA subr = {
        data,
        width * (UINT)texel_size,
        width * height * (UINT)texel_size,
    };

    ID3D11Texture2D *ret = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&desc, data ? &subr : nullptr, &ret);
    if (FAILED(hr))
    {
        return TranslateReturnCode(hr);
    }
    *dst_tex = ret;
    return Result::OK;
}

void GraphicsInterfaceD3D11::releaseTexture2D(void *tex)
{
    if (tex) {
        ((ID3D11Texture2D*)tex)->Release();
    }
}

Result GraphicsInterfaceD3D11::readTexture2D(void *dst, size_t read_size, void *src_tex_, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_tex_) { return Result::InvalidParameter; }

    auto *src_tex = (ID3D11Texture2D*)src_tex_;

    auto proc_read = [this](void *dst, size_t dst_size, ID3D11Texture2D *tex, int width, int height, TextureFormat format) -> HRESULT {
        D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
        auto hr = m_context->Map(tex, 0, D3D11_MAP_READ, 0, &mapped);
        if (FAILED(hr)) { return hr; }

        auto *dst_pixels = (char*)dst;
        auto *src_pixels = (const char*)mapped.pData;
        int dst_pitch = width * GetTexelSize(format);
        int src_pitch = mapped.RowPitch;
        int num_rows = std::min<int>(height, (int)ceildiv<size_t>(dst_size, dst_pitch));
        CopyRegion(dst_pixels, dst_pitch, src_pixels, src_pitch, num_rows);

        m_context->Unmap(tex, 0);
        return S_OK;
    };

    // try direct access
    auto hr = proc_read(dst, read_size, src_tex, width, height, format);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingTexture(width, height, format, StagingFlag::Readback);
    m_context->CopyResource(staging.Get(), src_tex);
    sync(); // Map() doesn't wait completion of above CopyResource(). manual synchronization is required.
    hr = proc_read(dst, read_size, staging.Get(), width, height, format);

    return TranslateReturnCode(hr);
}

Result GraphicsInterfaceD3D11::writeTexture2D(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_tex_ || !src) { return Result::InvalidParameter; }

    auto *dst_tex = (ID3D11Texture2D*)dst_tex_;

    auto proc_write = [this](ID3D11Texture2D *tex, int width, int height, TextureFormat format, const void *src, size_t write_size) -> HRESULT {
        D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
        auto hr = m_context->Map(tex, 0, D3D11_MAP_WRITE, 0, &mapped);
        if (FAILED(hr)) { return hr; }

        auto *dst_pixels = (char*)mapped.pData;
        auto *src_pixels = (const char*)src;
        int dst_pitch = mapped.RowPitch;
        int src_pitch = width * GetTexelSize(format);
        int num_rows = std::min<int>(height, (int)ceildiv<size_t>(write_size, src_pitch));
        CopyRegion(dst_pixels, dst_pitch, src_pixels, src_pitch, num_rows);

        m_context->Unmap(tex, 0);
        return S_OK;
    };


    // try direct access
    auto hr = proc_write(dst_tex, width, height, format, src, write_size);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingTexture(width, height, format, StagingFlag::Upload);
    hr = proc_write(staging.Get(), width, height, format, src, write_size);
    m_context->CopyResource(dst_tex, staging.Get());

    return TranslateReturnCode(hr);
}



Result GraphicsInterfaceD3D11::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    D3D11_BUFFER_DESC desc = { 0 };
    desc.ByteWidth = (UINT)size;
    desc.Usage = D3D11_USAGE_DEFAULT;

    switch (type) {
    case BufferType::Index:
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        break;
    case BufferType::Vertex:
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        break;
    case BufferType::Constant:
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        break;
    case BufferType::Compute:
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        break;
    }

    desc.CPUAccessFlags = 0;
    if (flags & ResourceFlags::CPU_Write) {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }
    if (flags & ResourceFlags::CPU_Read) {
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
    }

    D3D11_SUBRESOURCE_DATA subr = {
        data,
        (UINT)size,
        1,
    };

    ID3D11Buffer *ret = nullptr;
    HRESULT hr = m_device->CreateBuffer(&desc, data ? &subr : nullptr, &ret);
    if (FAILED(hr)) {
        return TranslateReturnCode(hr);
    }
    *dst_buf = ret;
    return Result::OK;
}

void GraphicsInterfaceD3D11::releaseBuffer(void *buf)
{
    if (buf) {
        ((ID3D11Buffer*)buf)->Release();
    }
}

Result GraphicsInterfaceD3D11::readBuffer(void *dst, void *src_buf_, size_t read_size, BufferType type)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_buf_) { return Result::InvalidParameter; }

    auto *src_buf = (ID3D11Buffer*)src_buf_;


    auto proc_read = [this](void *dst, ID3D11Buffer *buf, size_t size) -> HRESULT {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = m_context->Map(buf, 0, D3D11_MAP_READ, 0, &mapped);
        if (FAILED(hr)) { return hr; }
        memcpy(dst, mapped.pData, size);
        m_context->Unmap(buf, 0);
        return S_OK;
    };

    // try direct access
    auto hr = proc_read(dst, src_buf, read_size);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingBuffer(read_size, StagingFlag::Readback);
    m_context->CopyResource(staging.Get(), src_buf);
    sync();
    hr = proc_read(dst, staging.Get(), read_size);

    return TranslateReturnCode(hr);
}

Result GraphicsInterfaceD3D11::writeBuffer(void *dst_buf_, const void *src, size_t write_size, BufferType type)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_buf_ || !src) { return Result::InvalidParameter; }

    auto *dst_buf = (ID3D11Buffer*)dst_buf_;

    auto proc_write = [this](ID3D11Buffer *buf, const void *src, size_t size) -> HRESULT {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = m_context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) { return hr; }
        memcpy(mapped.pData, src, size);
        m_context->Unmap(buf, 0);
        return S_OK;
    };

    // try direct access
    auto hr = proc_write(dst_buf, src, write_size);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingBuffer(write_size, StagingFlag::Upload);
    hr = proc_write(staging.Get(), src, write_size);
    m_context->CopyResource(dst_buf, staging.Get());

    return TranslateReturnCode(hr);
}

} // namespace gi
#endif // giSupportD3D11
