#include "rpp.h"
#include <array>
#include <vector>
#include <iostream>


template <class _Ty>
concept test_concept = std::copyable<_Ty> && std::movable<_Ty>;
    // && requires(_Ty __i) {
    //     typename iter_difference_t<_Ty>;
    //     requires _Signed_integer_like<iter_difference_t<_Ty>>;
    //     { ++__i } -> same_as<_Ty&>;
    //     __i++;
    // };



template <typename T>
void jhfds(T)
{}
int main()
{
    using namespace rpp::conv;
    // static_assert(test_concept<decltype(utf8)>);
    auto v = std::string{"héllo"};
    auto t = static_cast<const std::string&>(v) | rpp::conv::from(utf8);
    std::ranges::begin(t);
    // auto v = std::vector{1,2,3,4};
    // auto t = v | rpp::plus_one;
    // std::ranges::begin(t);



    // auto v = std::string("Héllöworld");
    // auto t = static_cast<const std::string&>(v) | from(utf8);// | to(utf8);
    // auto t1 = t.begin();
    // auto t2 = t.begin();
    // auto t3 = std::move(t1);
    // std::ranges::swap(t1, t2);
    // jhfds(t.begin());
    // auto test = std::ranges::begin(t);

    // auto t = static_cast<const std::string&>(v) | std::views::transform([](char c) { return c; });

    for(auto i : t)
    {
        std::cout << std::dec<<i << '(' << std::hex << i << ") ";
        if (i<0x80)
            std::cout <<(char)i;
        std::cout << std::endl;
    }
    return 0;
}