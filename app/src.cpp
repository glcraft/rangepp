#include "rpp.h"
#include <array>
#include <vector>
#include <iostream>
#include <concepts>


template <class _Ty>
concept test_concept = std::copyable<_Ty> && std::movable<_Ty>;
    // && requires(_Ty __i) {
    //     typename iter_difference_t<_Ty>;
    //     requires _Signed_integer_like<iter_difference_t<_Ty>>;
    //     { ++__i } -> same_as<_Ty&>;
    //     __i++;
    // };
template <typename T>
auto unsigned_cast(T v);
template <std::unsigned_integral T>
auto unsigned_cast(T v)
{ return v; }
template <>
auto unsigned_cast<int8_t>(int8_t v)
{ return static_cast<uint8_t>(v); }
template <>
auto unsigned_cast<int16_t>(int16_t v)
{ return static_cast<uint16_t>(v); }
template <>
auto unsigned_cast<int32_t>(int32_t v)
{ return static_cast<uint32_t>(v); }
template <>
auto unsigned_cast<int64_t>(int64_t v)
{ return static_cast<uint64_t>(v); }
template <>
auto unsigned_cast<char8_t>(char8_t v)
{ return static_cast<uint8_t>(v); }
template <>
auto unsigned_cast<char16_t>(char16_t v)
{ return static_cast<uint16_t>(v); }
template <>
auto unsigned_cast<char32_t>(char32_t v)
{ return static_cast<uint32_t>(v); }

template <typename T>
void jhfds(T)
{}
int main()
{
    using namespace rpp::conv;
    using namespace std::literals;
    auto t = u"h\xe9llo\x305d\x308c\x306f\x30c6\x30b9\x30c8\x3067\x3059"sv 
        | from<utf16be>
        | to<utf32>;
    for (auto it = t.begin(); it!=t.end();++it)
    {
        auto i = *it;
        auto c = static_cast<char>(i);
        
        auto v = unsigned_cast(i);
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