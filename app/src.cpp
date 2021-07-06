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
    auto t = std::string_view("hélloà") | from(utf8)| to(utf8);

    for(auto i : t)
    {
        auto c = static_cast<char>(i);
        auto v = static_cast<int>(i);
        std::cout << std::dec << v << '(' << std::hex << v << ") ";
        if (v<0x80)
            std::cout <<c;
        std::cout << std::endl;
    }
    return 0;
}