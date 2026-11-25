#include "Window.hpp"

#include <system_error>

namespace edu::win32
{
namespace
{
constexpr wchar_t windowClassName[] = L"EduDx12Engine.Window";
constexpr DWORD windowStyle = WS_OVERLAPPEDWINDOW;

[[noreturn]] void ThrowWin32Error(const char* operation, DWORD error)
{
    throw std::system_error(static_cast<int>(error), std::system_category(), operation);
}
}

Window::Window(const wchar_t* title, ClientAreaChanged onClientAreaChanged)
    : instance_(GetModuleHandleW(nullptr)), onClientAreaChanged_(onClientAreaChanged)
{
    if (!instance_)
    {
        ThrowWin32Error("GetModuleHandleW", GetLastError());
    }

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // クライアント領域の色は描画側が設定する。
    windowClass.lpszClassName = windowClassName;
    if (!RegisterClassW(&windowClass))
    {
        ThrowWin32Error("RegisterClassW", GetLastError());
    }

    handle_ = CreateWindowExW(0, windowClassName, title, windowStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 450,
                             nullptr, nullptr, instance_, this);
    if (!handle_)
    {
        // コンストラクタが失敗するとデストラクタは呼ばれない。
        const DWORD error = GetLastError();
        UnregisterClassW(windowClassName, instance_);
        ThrowWin32Error("CreateWindowExW", error);
    }
}

Window::~Window()
{
    if (handle_)
    {
        DestroyWindow(handle_);
    }
    UnregisterClassW(windowClassName, instance_);
}

void Window::Show()
{
    // 初回も、枠やタイトルバーを除いた実際の寸法を知らせる。
    RECT area{};
    if (!GetClientRect(handle_, &area))
    {
        ThrowWin32Error("GetClientRect", GetLastError());
    }
    UpdateClientArea({area.right, area.bottom, false});
    ShowWindow(handle_, SW_SHOWNORMAL);
}

std::optional<int> Window::ProcessMessages()
{
    MSG message{};
    // WM_QUITはウィンドウ宛てではないため、HWNDでは絞り込まない。
    while (!closeRequested_ && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            return static_cast<int>(message.wParam);
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (closeRequested_)
    {
        return 0;
    }
    return std::nullopt;
}

void Window::WaitForMessage()
{
    if (!WaitMessage())
    {
        ThrowWin32Error("WaitMessage", GetLastError());
    }
}

LRESULT CALLBACK Window::WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    if (message == WM_NCCREATE)
    {
        // CreateWindowExWが渡した非所有ポインタで、通知先のWindowを特定する。
        const auto* creation = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        SetLastError(ERROR_SUCCESS);
        if (SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(creation->lpCreateParams)) == 0 &&
            GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
        // タイトルなどの標準処理は、関連付けが済んだ後もWindowsに任せる。
        return DefWindowProcW(handle, message, wParam, lParam);
    }

    auto* window = reinterpret_cast<Window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
    if (window && window->handle_)
    {
        switch (message)
        {
        case WM_CLOSE:
            // swap chainを解放してからHWNDを破棄できるよう、ここでは要求だけを記録する。
            window->closeRequested_ = true;
            return 0;
        case WM_SIZE:
            // 最小化時は利用できる描画領域がないことを0×0で伝える。
            if (wParam == SIZE_MINIMIZED)
            {
                window->UpdateClientArea({0, 0, true});
            }
            else if (RECT area{}; GetClientRect(handle, &area))
            {
                window->UpdateClientArea({area.right, area.bottom, false});
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_NCDESTROY:
            SetWindowLongPtrW(handle, GWLP_USERDATA, 0);
            window->handle_ = nullptr;
            break;
        }
    }

    // タイトルバーなど、個別に扱わない通知はWindowsの既定処理へ渡す。
    return DefWindowProcW(handle, message, wParam, lParam);
}

void Window::UpdateClientArea(ClientArea area) noexcept
{
    if (area == clientArea_)
    {
        return;
    }

    clientArea_ = area;
    if (onClientAreaChanged_)
    {
        onClientAreaChanged_(area);
    }
}
}
