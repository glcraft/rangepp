#include "rangepp.h"
#include <array>
#include <vector>
#include <iostream>
int main()
{
    auto v = std::array{1,2,3};
    auto t = v | rpp::plus_one;
    // auto t = v | std::views::transform([](int i) { return i+1; });
    // std::vector<int> v2{std::ranges::begin(t), std::ranges::end(t)};
    for(auto i : t)
        std::cout << i << std::endl;
    return 0;
}