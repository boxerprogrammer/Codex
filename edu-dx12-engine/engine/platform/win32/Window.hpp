#pragma once

#include <windows.h>
#include <optional>

namespace edu::win32
{
struct ClientArea
{
    int width{};
    int height{};
    bool minimized{};

    bool operator==(const ClientArea&) const = default;
};

// 生成・表示・メッセージ処理・破棄はすべて同じスレッドで行う。
// この段階では一つのアプリに一つのウィンドウを置く。
class Window
{
public:
    using ClientAreaChanged = void (*)(ClientArea) noexcept;

    Window(const wchar_t* title, ClientAreaChanged onClientAreaChanged);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    // OSがこのオブジェクトのアドレスを覚えているため、移動もしない。
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void Show();
    [[nodiscard]] HWND NativeHandle() const noexcept { return handle_; }
    [[nodiscard]] ClientArea GetClientArea() const noexcept { return clientArea_; }

    // 閉じる要求またはWM_QUITを受け取ると終了コードを返す。
    [[nodiscard]] std::optional<int> ProcessMessages();
    void WaitForMessage();

private:
    static LRESULT CALLBACK WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    void UpdateClientArea(ClientArea area) noexcept;

    HINSTANCE instance_{};
    HWND handle_{};
    ClientArea clientArea_{};
    ClientAreaChanged onClientAreaChanged_{};
    bool closeRequested_{};
};
}
