#include "pch.h"
#include "giInternal.h"

#ifdef giSupportD3D12
#include <d3d12.h>
#include <d3dx12.h>

using Microsoft::WRL::ComPtr;

namespace gi {

class GraphicsInterfaceD3D12 : public GraphicsInterface
{
public:
    GraphicsInterfaceD3D12(void *device);
    ~GraphicsInterfaceD3D12();
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void   releaseTexture2D(void *tex) override;
    Result readTexture2D(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void   releaseBuffer(void *buf) override;
    Result readBuffer(void *dst, void *src_buf, size_t read_size, BufferType type) override;
    Result writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
    enum class StagingFlag {
        Upload,
        Readback,
    };
    ComPtr<ID3D12Resource> createStagingBuffer(size_t size, StagingFlag flag);

    // Body: [](ID3D12GraphicsCommandList *clist) -> void
    template<class Body> HRESULT executeCommands(const Body& body);

private:
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_cqueue;
    ComPtr<ID3D12CommandAllocator> m_calloc;
    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fence_value = 0;
    HANDLE m_fence_event;
};


GraphicsInterface* CreateGraphicsInterfaceD3D12(void *device)
{
    if (!device) { return nullptr; }
    return new GraphicsInterfaceD3D12(device);
}


GraphicsInterfaceD3D12::GraphicsInterfaceD3D12(void *device)
    : m_device((ID3D12Device*)device)
{
    // create command queue
    {
        D3D12_COMMAND_QUEUE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        auto hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cqueue));
    }

    // create command allocator & list
    {
        auto hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_calloc));
    }

    // create signal
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fence->Signal(1);

    m_fence_event = CreateEvent(nullptr, false, false, nullptr);
}

GraphicsInterfaceD3D12::~GraphicsInterfaceD3D12()
{
    if (m_fence_event) {
        CloseHandle(m_fence_event);
    }
}

void GraphicsInterfaceD3D12::release()
{
    delete this;
}

void* GraphicsInterfaceD3D12::getDevicePtr()
{
    return nullptr;
}

DeviceType GraphicsInterfaceD3D12::getDeviceType()
{
    return DeviceType::D3D12;
}

// Body: [](ID3D12GraphicsCommandList *clist) -> void
template<class Body>
HRESULT GraphicsInterfaceD3D12::executeCommands(const Body& body)
{
    HRESULT hr;

    ComPtr<ID3D12GraphicsCommandList> clist;
    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_calloc.Get(), nullptr, IID_PPV_ARGS(&clist));
    if (FAILED(hr)) { return hr; }

    body(clist.Get());

    hr = clist->Close();
    if (FAILED(hr)) { return hr; }

    m_cqueue->ExecuteCommandLists(1, (ID3D12CommandList**)clist.GetAddressOf());

    ++m_fence_value;
    hr = m_cqueue->Signal(m_fence.Get(), m_fence_value);
    if (FAILED(hr)) { return hr; }

    hr = m_fence->SetEventOnCompletion(m_fence_value, m_fence_event);
    if (FAILED(hr)) { return hr; }

    WaitForSingleObject(m_fence_event, INFINITE);

    return S_OK;
}


void GraphicsInterfaceD3D12::sync()
{
}

ComPtr<ID3D12Resource> GraphicsInterfaceD3D12::createStagingBuffer(size_t size, StagingFlag flag)
{
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ;

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type                   = D3D12_HEAP_TYPE_DEFAULT;
    heap.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    heap.CreationNodeMask       = 0;
    heap.VisibleNodeMask        = 0;
    if (flag == StagingFlag::Upload) {
        heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    }
    if (flag == StagingFlag::Readback) {
        state = D3D12_RESOURCE_STATE_COPY_DEST;
        heap.Type = D3D12_HEAP_TYPE_READBACK;
    }

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto ret = ComPtr<ID3D12Resource>();
    m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&ret));
    return ret;
}

Result GraphicsInterfaceD3D12::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    D3D12_HEAP_PROPERTIES heap  = {};
    heap.Type                   = D3D12_HEAP_TYPE_DEFAULT;
    heap.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    heap.CreationNodeMask       = 1;
    heap.VisibleNodeMask        = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = (UINT64)width;
    desc.Height             = (UINT)height;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = GetDXGIFormat(format);
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    // texture can't be created with D3D12_HEAP_TYPE_UPLOAD / D3D12_HEAP_TYPE_READBACK heap type.
    // ResourceFlags::CPU_Write / CPU_Read flag is ignored.


    ID3D12Resource *tex = nullptr;
    auto hr = m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&tex) );
    if ( FAILED( hr ) ) { return TranslateReturnCode(hr); }
    *dst_tex = tex;

    if (data) {
        writeTexture2D(tex, width, height, format, data, width * height * GetTexelSize(format));
    }

    return Result::OK;
}

void GraphicsInterfaceD3D12::releaseTexture2D(void *tex_)
{
    if (!tex_) { return; }

    auto *tex = (ID3D12Resource*)tex_;
    tex->Release();
}

Result GraphicsInterfaceD3D12::readTexture2D(void *dst, size_t read_size, void *src_tex_, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_tex_) { return Result::InvalidParameter; }

    auto *src_tex = (ID3D12Resource*)src_tex_;

    D3D12_RESOURCE_DESC src_desc = src_tex->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT src_layout;
    UINT src_num_rows;
    UINT64 src_row_size;
    UINT64 src_required_size;
    m_device->GetCopyableFootprints(&src_desc, 0, 1, 0, &src_layout, &src_num_rows, &src_row_size, &src_required_size);


    auto read_proc = [](void *dst, size_t read_size, ID3D12Resource *src_tex, int width, int height, TextureFormat format, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& src_layout) -> HRESULT {
        void *mapped_data = nullptr;
        auto hr = src_tex->Map(0, nullptr, &mapped_data);
        if (FAILED(hr)) { return hr; }

        int dst_pitch = width * GetTexelSize(format);
        int src_pitch = src_layout.Footprint.RowPitch;
        int num_rows = std::min<int>(std::min<int>(height, (int)src_layout.Footprint.Height), (int)ceildiv<size_t>(read_size, dst_pitch));
        CopyRegion(dst, dst_pitch, mapped_data, src_pitch, num_rows);
        src_tex->Unmap(0, nullptr);
        return S_OK;
    };


    // try direct access
    auto hr = read_proc(dst, read_size, src_tex, width, height, format, src_layout);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingBuffer(src_required_size, StagingFlag::Readback);
    if (!staging) { return Result::OutOfMemory; }

    hr = executeCommands([&](ID3D12GraphicsCommandList *clist) {
        CD3DX12_TEXTURE_COPY_LOCATION dst_region(staging.Get(), src_layout);
        CD3DX12_TEXTURE_COPY_LOCATION src_region(src_tex, 0);

        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src_tex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
        clist->CopyTextureRegion(&dst_region, 0, 0, 0, &src_region, nullptr);
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src_tex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
    });
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    hr = read_proc(dst, read_size, staging.Get(), width, height, format, src_layout);
    return TranslateReturnCode(hr);
}

Result GraphicsInterfaceD3D12::writeTexture2D(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_tex_ || !src) { return Result::InvalidParameter; }

    auto *dst_tex = (ID3D12Resource*)dst_tex_;

    D3D12_RESOURCE_DESC dst_desc = dst_tex->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT dst_layout;
    UINT dst_num_rows;
    UINT64 dst_row_size;
    UINT64 dst_required_size;
    m_device->GetCopyableFootprints(&dst_desc, 0, 1, 0, &dst_layout, &dst_num_rows, &dst_row_size, &dst_required_size);


    auto write_proc = [](ID3D12Resource *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& dst_layout) {
        void *mapped_data = nullptr;
        auto hr = dst_tex->Map(0, nullptr, &mapped_data);
        if (FAILED(hr)) { return hr; }

        int dst_pitch = dst_layout.Footprint.RowPitch;
        int src_pitch = width * GetTexelSize(format);
        int num_rows = std::min<int>(std::min<int>(height, (int)dst_layout.Footprint.Height), (int)ceildiv<size_t>(write_size, src_pitch));
        CopyRegion(mapped_data, dst_pitch, src, src_pitch, num_rows);
        dst_tex->Unmap(0, nullptr);
        return S_OK;
    };


    // try direct access
    auto hr = write_proc(dst_tex, width, height, format, src, write_size, dst_layout);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingBuffer(dst_required_size, StagingFlag::Upload);
    if (!staging) { return Result::OutOfMemory; }

    hr = write_proc(staging.Get(), width, height, format, src, write_size, dst_layout);
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    hr = executeCommands([&](ID3D12GraphicsCommandList *clist) {
        CD3DX12_TEXTURE_COPY_LOCATION dst_region(dst_tex, 0);
        CD3DX12_TEXTURE_COPY_LOCATION src_region(staging.Get(), dst_layout);

        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dst_tex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
        clist->CopyTextureRegion(&dst_region, 0, 0, 0, &src_region, nullptr);
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dst_tex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
    });
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    return Result::OK;
}



Result GraphicsInterfaceD3D12::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    if (!dst_buf) { return Result::InvalidParameter; }
    
    D3D12_HEAP_PROPERTIES heap = {};
     heap.Type                 = D3D12_HEAP_TYPE_DEFAULT;
     heap.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
     heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
     heap.CreationNodeMask     = 1;
     heap.VisibleNodeMask      = 1;

     D3D12_RESOURCE_DESC desc = {};
     desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
     desc.Alignment          = 0;
     desc.Width              = (UINT64)size;
     desc.Height             = 1;
     desc.DepthOrArraySize   = 1;
     desc.MipLevels          = 1;
     desc.Format             = DXGI_FORMAT_UNKNOWN;
     desc.SampleDesc.Count   = 1;
     desc.SampleDesc.Quality = 0;
     desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
     desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

     D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
     if (type == BufferType::Index)    { state |= D3D12_RESOURCE_STATE_INDEX_BUFFER; }
     if (type == BufferType::Vertex)   { state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; }
     if (type == BufferType::Constant) { state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; }
     if (type == BufferType::Compute)  {}

     if (flags & ResourceFlags::CPU_Write) {
         heap.Type = D3D12_HEAP_TYPE_UPLOAD;
     }
     if (flags & ResourceFlags::CPU_Read) {
         heap.Type = D3D12_HEAP_TYPE_READBACK;
     }


     ID3D12Resource *buf = nullptr;
     auto hr = m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&buf) );
     if (FAILED(hr)) { return TranslateReturnCode(hr); }
     *dst_buf = buf;

     if (data) {
         writeBuffer(buf, data, size, type);
     }

     return Result::OK;
}

void GraphicsInterfaceD3D12::releaseBuffer(void *buf_)
{
    if (!buf_) { return; }

    auto *buf = (ID3D12Resource*)buf_;
    buf->Release();
}

Result GraphicsInterfaceD3D12::readBuffer(void *dst, void *src_buf, size_t read_size, BufferType type)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_buf) { return Result::InvalidParameter; }

    auto *buf = (ID3D12Resource*)src_buf;

    auto read_proc = [](void *dst, ID3D12Resource *src_buf, size_t read_size) -> HRESULT {
        void *mapped_data = nullptr;
        auto hr = src_buf->Map(0, nullptr, &mapped_data);
        if (FAILED(hr)) { return hr; }
        memcpy(dst, mapped_data, read_size);
        src_buf->Unmap(0, nullptr);
        return S_OK;
    };


    // try direct access
    auto hr = read_proc(dst, buf, read_size);
    if (SUCCEEDED(hr)) { return Result::OK; }

    // try copy-via-staging
    auto staging = createStagingBuffer(read_size, StagingFlag::Readback);
    if (!staging) { return Result::OutOfMemory; }

    hr = executeCommands([&](ID3D12GraphicsCommandList *clist) {
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buf, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
        clist->CopyResource(staging.Get(), buf);
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buf, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
    });
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    hr = read_proc(dst, staging.Get(), read_size);
    return TranslateReturnCode(hr);
}

Result GraphicsInterfaceD3D12::writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_buf || !src) { return Result::InvalidParameter; }

    auto *buf = (ID3D12Resource*)dst_buf;

    auto write_proc = [](ID3D12Resource *buf, const void *src, size_t write_size) -> HRESULT {
        void *mapped_data = nullptr;
        auto hr = buf->Map(0, nullptr, &mapped_data);
        if (FAILED(hr)) { return hr; }
        memcpy(mapped_data, src, write_size);
        buf->Unmap(0, nullptr);
        return hr;
    };


    // try direct access
    auto hr = write_proc(buf, src, write_size);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingBuffer(write_size, StagingFlag::Upload);
    if (!staging) { return Result::OutOfMemory; }

    hr = write_proc(staging.Get(), src, write_size);
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    hr = executeCommands([&](ID3D12GraphicsCommandList *clist) {
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buf, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
        clist->CopyResource(buf, staging.Get());
        clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buf, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
    });
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    return Result::OK;
}

} // namespace gi
#endif // giSupportD3D12
