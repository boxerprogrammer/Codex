#pragma once

#include <windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <array>
#include <memory>

namespace edu::render
{
class ClearRenderer
{
public:
    using Color = std::array<float, 4>;

    // windowは借り物。呼び出し側で、この描画クラスより長く生かす。
    ClearRenderer(HWND window, UINT width, UINT height);
    ~ClearRenderer();

    ClearRenderer(const ClearRenderer&) = delete;
    ClearRenderer& operator=(const ClearRenderer&) = delete;

    void Resize(UINT width, UINT height);
    void Draw(const Color& color);

private:
    struct DebugMessageLog
    {
        void Start(ID3D12Device* device);
        ~DebugMessageLog();

        Microsoft::WRL::ComPtr<ID3D12InfoQueue1> queue;
        DWORD cookie{};
    };

    void CreateBackBuffers();
    void WaitForGpu();
    void CheckResult(HRESULT result, const char* operation) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget(UINT index) const noexcept;

    static constexpr UINT bufferCount = 2;
    static constexpr DXGI_FORMAT bufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    DebugMessageLog debugLog_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTargets_;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, bufferCount> backBuffers_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    std::unique_ptr<void, decltype(&CloseHandle)> fenceEvent_{nullptr, CloseHandle};

    UINT descriptorSize_{};
    UINT width_{};
    UINT height_{};
    UINT64 fenceValue_{};
    bool gpuWorkPending_{};
};
}
