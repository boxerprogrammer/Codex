#include "ClearRenderer.hpp"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <limits>
#include <stdexcept>
#include <system_error>

namespace edu::render
{
using Microsoft::WRL::ComPtr;

namespace
{
void CALLBACK LogDebugMessage(D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY severity,
                              D3D12_MESSAGE_ID id, const char* description, void*) noexcept
{
    if (severity <= D3D12_MESSAGE_SEVERITY_WARNING)
    {
        std::fprintf(stderr, "D3D12 Debug Layer [severity %d, ID %d] %s\n",
                     static_cast<int>(severity), static_cast<int>(id), description);
    }
}

[[noreturn]] void ThrowDxError(HRESULT result, const char* operation, HRESULT removedReason = S_OK)
{
    char message[256]{};
    std::snprintf(message, sizeof(message), "%s failed (HRESULT 0x%08lX, device status 0x%08lX).",
                  operation, static_cast<unsigned long>(result), static_cast<unsigned long>(removedReason));
    throw std::runtime_error(message);
}
}

ClearRenderer::ClearRenderer(HWND window, UINT width, UINT height)
    : width_(width), height_(height)
{
    if (width == 0 || height == 0)
    {
        throw std::invalid_argument("The initial client area must be nonzero.");
    }

#ifndef NDEBUG
    ComPtr<ID3D12Debug> debug;
    CheckResult(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)), "D3D12GetDebugInterface (check Graphics Tools)");
    debug->EnableDebugLayer();
#endif

    ComPtr<IDXGIFactory4> factory;
    CheckResult(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "CreateDXGIFactory2");
    HRESULT deviceResult = DXGI_ERROR_UNSUPPORTED;
    for (UINT index = 0; ; ++index)
    {
        ComPtr<IDXGIAdapter1> adapter;
        const HRESULT enumResult = factory->EnumAdapters1(index, &adapter);
        if (enumResult == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }
        CheckResult(enumResult, "EnumAdapters1");

        DXGI_ADAPTER_DESC1 description{};
        CheckResult(adapter->GetDesc1(&description), "GetDesc1");
        if ((description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        {
            continue;
        }
        deviceResult = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
        if (SUCCEEDED(deviceResult))
        {
            std::printf("D3D12 hardware adapter: %ls (%04X:%04X)\n",
                        description.Description, description.VendorId, description.DeviceId);
            break;
        }
    }
    CheckResult(deviceResult, "D3D12CreateDevice (hardware adapter)");

#ifndef NDEBUG
    debugLog_.Start(device_.Get());
    std::puts("D3D12 Debug Layer: enabled (warning, error, and corruption logging).");
#endif

    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CheckResult(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)), "CreateCommandQueue");
    CheckResult(commandQueue_->SetName(L"Clear direct queue"), "SetName (queue)");

    DXGI_SWAP_CHAIN_DESC1 swapDesc{};
    swapDesc.Width = width_;
    swapDesc.Height = height_;
    swapDesc.Format = bufferFormat;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = bufferCount;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swapChain;
    // D3D12のPresentはデバイスではなく、描画命令と同じdirect queueへ渡す。
    CheckResult(factory->CreateSwapChainForHwnd(commandQueue_.Get(), window, &swapDesc,
                                                nullptr, nullptr, &swapChain), "CreateSwapChainForHwnd");
    CheckResult(swapChain.As(&swapChain_), "QueryInterface (IDXGISwapChain3)");
    CheckResult(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = bufferCount;
    CheckResult(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&renderTargets_)), "CreateDescriptorHeap (RTV)");
    descriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CreateBackBuffers();
    std::printf("Swap chain: %u x %u\n", width_, height_);
    std::fflush(stdout);

    CheckResult(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_)),
                "CreateCommandAllocator");
    CheckResult(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(),
                                           nullptr, IID_PPV_ARGS(&commandList_)), "CreateCommandList");
    CheckResult(commandList_->SetName(L"Clear commands"), "SetName (command list)");
    // 作成直後の記録リストは開いている。初回もResetから始められるように閉じる。
    CheckResult(commandList_->Close(), "Close (initial command list)");
    CheckResult(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)), "CreateFence");
    fenceEvent_.reset(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (!fenceEvent_)
    {
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "CreateEventW (fence)");
    }
}

ClearRenderer::~ClearRenderer()
{
    try
    {
        WaitForGpu();
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "D3D12 shutdown: %s\n", error.what());
        if (SUCCEEDED(device_->GetDeviceRemovedReason()))
        {
            // 完了を確認できない場合は、GPUが使用中かもしれない資産を個別に解放しない。
            std::fflush(stderr);
            std::quick_exit(EXIT_FAILURE);
        }
    }
}

void ClearRenderer::Resize(UINT width, UINT height)
{
    if (width == 0 || height == 0 || (width == width_ && height == height_))
    {
        return;
    }

    WaitForGpu();
    // ResizeBuffersの前に、GPUの完了と旧バッファのCPU参照の解放が両方必要。
    for (auto& buffer : backBuffers_)
    {
        buffer.Reset();
    }
    CheckResult(swapChain_->ResizeBuffers(bufferCount, width, height, bufferFormat, 0), "ResizeBuffers");
    CreateBackBuffers();
    width_ = width;
    height_ = height;
    std::printf("Swap chain: %u x %u\n", width_, height_);
    std::fflush(stdout);
}

void ClearRenderer::Draw(const Color& color)
{
    // 前回のDrawはfence完了まで待つため、命令の記録領域を一つだけ再利用できる。
    CheckResult(commandAllocator_->Reset(), "Reset (command allocator)");
    CheckResult(commandList_->Reset(commandAllocator_.Get(), nullptr), "Reset (command list)");

    const UINT index = swapChain_->GetCurrentBackBufferIndex();
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = backBuffers_[index].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList_->ResourceBarrier(1, &barrier);

    commandList_->ClearRenderTargetView(RenderTarget(index), color.data(), 0, nullptr);

    // DXGIが画面へ渡せるよう、色を書いた後のバッファをPRESENTへ戻す。
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList_->ResourceBarrier(1, &barrier);
    CheckResult(commandList_->Close(), "Close (clear commands)");

    ID3D12CommandList* commands[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(1, commands);
    gpuWorkPending_ = true;
    CheckResult(swapChain_->Present(1, 0), "Present");
    WaitForGpu();
}

void ClearRenderer::CreateBackBuffers()
{
    constexpr const wchar_t* names[] = {L"Clear back buffer 0", L"Clear back buffer 1"};
    for (UINT index = 0; index < bufferCount; ++index)
    {
        CheckResult(swapChain_->GetBuffer(index, IID_PPV_ARGS(&backBuffers_[index])), "GetBuffer");
        CheckResult(backBuffers_[index]->SetName(names[index]), "SetName (back buffer)");
        device_->CreateRenderTargetView(backBuffers_[index].Get(), nullptr, RenderTarget(index));
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE ClearRenderer::RenderTarget(UINT index) const noexcept
{
    auto handle = renderTargets_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * descriptorSize_;
    return handle;
}

void ClearRenderer::WaitForGpu()
{
    if (!gpuWorkPending_)
    {
        return;
    }

    CheckResult(device_->GetDeviceRemovedReason(), "GetDeviceRemovedReason");
    const UINT64 target = ++fenceValue_;
    CheckResult(commandQueue_->Signal(fence_.Get(), target), "Signal (fence)");
    constexpr DWORD waitMs = 5000;
    for (;;)
    {
        const UINT64 completed = fence_->GetCompletedValue();
        if (completed == std::numeric_limits<UINT64>::max())
        {
            CheckResult(device_->GetDeviceRemovedReason(), "GetDeviceRemovedReason (fence)");
            throw std::runtime_error("The GPU fence reports a removed device.");
        }
        if (completed >= target)
        {
            gpuWorkPending_ = false;
            return;
        }

        CheckResult(fence_->SetEventOnCompletion(target, fenceEvent_.get()), "SetEventOnCompletion");
        const DWORD waitResult = WaitForSingleObject(fenceEvent_.get(), waitMs);
        if (waitResult == WAIT_FAILED)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "WaitForSingleObject (fence)");
        }
        if (waitResult != WAIT_OBJECT_0)
        {
            CheckResult(device_->GetDeviceRemovedReason(), "GetDeviceRemovedReason (fence timeout)");
            throw std::runtime_error("The GPU fence did not complete within five seconds.");
        }
    }
}

void ClearRenderer::CheckResult(HRESULT result, const char* operation) const
{
    if (FAILED(result))
    {
        ThrowDxError(result, operation, device_ ? device_->GetDeviceRemovedReason() : S_OK);
    }
}

void ClearRenderer::DebugMessageLog::Start(ID3D12Device* device)
{
    ComPtr<ID3D12InfoQueue1> infoQueue;
    if (const HRESULT result = device->QueryInterface(IID_PPV_ARGS(&infoQueue)); FAILED(result))
    {
        ThrowDxError(result, "QueryInterface (ID3D12InfoQueue1)");
    }
    if (const HRESULT result = infoQueue->RegisterMessageCallback(LogDebugMessage, D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                                                                  nullptr, &cookie); FAILED(result))
    {
        ThrowDxError(result, "RegisterMessageCallback");
    }
    queue = infoQueue;
}

ClearRenderer::DebugMessageLog::~DebugMessageLog()
{
    if (queue)
    {
        queue->UnregisterMessageCallback(cookie);
    }
}
}
