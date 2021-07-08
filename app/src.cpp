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
    using namespace std::literals;
    auto t = u"h\xe9llo\x305d\x308c\x306f\x30c6\x30b9\x30c8\x3067\x3059"sv 
        | from<utf16be>
        | to<utf8>;
    for (auto it = t.begin(); it!=t.end();++it)
    {
        auto i = *it;
        auto c = static_cast<char>(i);
        auto v = static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(&i));
        std::cout << std::dec << v << '(' << std::hex << v << ") ";
        if (v<0x80)
            std::cout <<c;
        std::cout << std::endl;
    }
    for(auto i : t)
        std::cout.put(i);
    std::cout << "\n";
    return 0;
}