#include <cstdlib>
#include <iostream>

// 言語モードの設定漏れを、実行前のコンパイルで検出する。
static_assert(__cplusplus >= 202002L, "This program requires C++20 or later.");

int main()
{
    std::cout << "C++ language version: " << __cplusplus << std::endl;
    return std::cout ? EXIT_SUCCESS : EXIT_FAILURE;
}
