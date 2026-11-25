#include "ClearRenderer.hpp"
#include "Window.hpp"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string_view>

namespace
{
void LogClientArea(edu::win32::ClientArea area) noexcept
{
    std::printf("Client area: %d x %d (%s)\n",
                area.width, area.height, area.minimized ? "minimized" : "active");
    std::fflush(stdout);
}
}

int main(int argc, char* argv[])
{
    const bool warm = argc == 2 && std::string_view(argv[1]) == "--warm";
    if (argc != 1 && !warm)
    {
        std::fputs("Usage: clear_window [--warm]\n", stderr);
        return EXIT_FAILURE;
    }

    // UNORMの描画先へ、そのまま表示するRGBA値を渡す。
    constexpr edu::render::ClearRenderer::Color coolColor{0.08f, 0.22f, 0.45f, 1.0f};
    constexpr edu::render::ClearRenderer::Color warmColor{0.65f, 0.24f, 0.10f, 1.0f};

    try
    {
        edu::win32::Window window(L"Edu DX12 Engine - M02", LogClientArea);
        window.Show();
        const auto initialArea = window.GetClientArea();
        int exitCode{};
        {
            // HWNDを使う描画側を、ウィンドウより先に破棄する。
            edu::render::ClearRenderer renderer(window.NativeHandle(),
                                                static_cast<UINT>(initialArea.width),
                                                static_cast<UINT>(initialArea.height));
            for (;;)
            {
                if (const auto exit = window.ProcessMessages())
                {
                    exitCode = *exit;
                    break;
                }

                const auto area = window.GetClientArea();
                if (area.minimized || area.width <= 0 || area.height <= 0)
                {
                    window.WaitForMessage();
                    continue;
                }

                renderer.Resize(static_cast<UINT>(area.width), static_cast<UINT>(area.height));
                renderer.Draw(warm ? warmColor : coolColor);
            }
        }
        std::printf("Renderer stopped. Message loop finished with exit code %d.\n", exitCode);
        return exitCode;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "clear_window: %s\n", error.what());
        return EXIT_FAILURE;
    }
}
