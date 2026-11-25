cmake_minimum_required(VERSION 3.21)
project(EduDx12Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# CPU側は標準C++だけでビルドし、Windows SDKの描画ヘッダやライブラリを持ち込まない。
add_executable(edu_environment_check samples/environment_check/main.cpp)
if(MSVC)
    target_compile_options(edu_environment_check PRIVATE /W4 /Zc:__cplusplus /utf-8)
endif()
add_test(NAME environment_check COMMAND edu_environment_check)

if(WIN32)
    add_library(edu_win32 STATIC engine/platform/win32/Window.cpp)
    target_include_directories(edu_win32 PUBLIC engine/platform/win32)
    target_compile_definitions(edu_win32 PUBLIC UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(edu_win32 PRIVATE user32)

    if(MSVC)
        target_compile_options(edu_win32 PRIVATE /W4 /utf-8)
    endif()

    option(EDU_CHECK_WINDOWS_SDK "Check the D3D12 and DXGI SDK and build the DirectX 12 sample" ON)
    if(EDU_CHECK_WINDOWS_SDK)
        include(CheckCXXSourceCompiles)
        include(CMakePushCheckState)

        # SDKのヘッダとライブラリを使って実行ファイルをリンクできるか確認する。このコードは実行しない。
        # volatileで関数への参照を残し、リンク時に両方の関数を解決させる。
        cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_LIBRARIES d3d12 dxgi)
        check_cxx_source_compiles([=[
            #include <windows.h>
            #include <d3d12.h>
            #include <dxgi1_6.h>

            int main()
            {
                auto* volatile createDevice = &D3D12CreateDevice;
                auto* volatile createFactory = &CreateDXGIFactory2;
                return createDevice && createFactory ? 0 : 1;
            }
        ]=] EDU_WINDOWS_SDK_CAN_LINK)
        cmake_pop_check_state()

        if(NOT EDU_WINDOWS_SDK_CAN_LINK)
            message(FATAL_ERROR
                "Could not compile and link with the Windows SDK headers and libraries for "
                "D3D12 and DXGI. Check the selected Windows SDK. "
                "For CPU-only verification, configure with -DEDU_CHECK_WINDOWS_SDK=OFF.")
        endif()

        add_library(edu_d3d12 STATIC engine/render/d3d12/ClearRenderer.cpp)
        target_include_directories(edu_d3d12 PUBLIC engine/render/d3d12)
        target_compile_definitions(edu_d3d12 PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        target_link_libraries(edu_d3d12 PRIVATE d3d12 dxgi)

        add_executable(clear_window samples/clear_window/main.cpp)
        target_link_libraries(clear_window PRIVATE edu_win32 edu_d3d12)
        if(MSVC)
            target_compile_options(edu_d3d12 PRIVATE /W4 /utf-8)
            target_compile_options(clear_window PRIVATE /W4 /utf-8)
        endif()
    endif()
endif()
