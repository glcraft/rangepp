#include "rangepp.h"
#include <array>
#include <vector>
int main()
{
    auto v = std::array{1,2,3};
    auto t = v | rpp::plus_one;
    // auto t = v | std::views::all| std::views::transform([](int i) { return i+1; });
    auto v2 = std::vector{t.begin(), t.end()};
    return 0;
}