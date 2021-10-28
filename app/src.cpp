#include "rpp.h"
#include <array>
#include <vector>
#include <iostream>
#include <concepts>
#include <cassert>
#include <chrono>
#ifdef __cpp_lib_format
#include <format>
#endif

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

void tests()
{
    using namespace std;
    auto cmp_iters = [](auto it1_begin, auto it1_end, auto it2_begin, auto it2_end) -> bool {
        for (;it1_begin!=it1_end && it2_begin != it2_end;++it2_begin, ++it1_begin) {
            auto v1 = *it1_begin;
            auto v2 = *it2_begin;
            if (v1 != v2)
                return false;
        }
        return it1_begin==it1_end && it2_begin == it2_end;
    };
    using namespace rpp::conv;
    using namespace std::literals;
    auto hello = array{char(0x68), char(0xC3), char(0xA9), char(0x6C), char(0x6C), char(0x6F), char(0xE3), char(0x81), char(0x9D), char(0xE3), char(0x82), char(0x8C), char(0xE3), char(0x81), char(0xAF), char(0xE3), char(0x83), char(0x86), char(0xE3), char(0x82), char(0xB9), char(0xE3), char(0x83), char(0x88), char(0xE3), char(0x81), char(0xA7), char(0xE3), char(0x81), char(0x99)};
    auto u8hello = array{char8_t(0x68), char8_t(0xC3), char8_t(0xA9), char8_t(0x6C), char8_t(0x6C), char8_t(0x6F), char8_t(0xE3), char8_t(0x81), char8_t(0x9D), char8_t(0xE3), char8_t(0x82), char8_t(0x8C), char8_t(0xE3), char8_t(0x81), char8_t(0xAF), char8_t(0xE3), char8_t(0x83), char8_t(0x86), char8_t(0xE3), char8_t(0x82), char8_t(0xB9), char8_t(0xE3), char8_t(0x83), char8_t(0x88), char8_t(0xE3), char8_t(0x81), char8_t(0xA7), char8_t(0xE3), char8_t(0x81), char8_t(0x99)};
    auto u16behello = array{char16_t(0x0068), char16_t(0x00E9), char16_t(0x006C), char16_t(0x006C), char16_t(0x006F), char16_t(0x305D), char16_t(0x308C), char16_t(0x306F), char16_t(0x30C6), char16_t(0x30B9), char16_t(0x30C8), char16_t(0x3067), char16_t(0x3059)};
    auto u16lehello = array{char16_t(0x6800), char16_t(0xE900), char16_t(0x6C00), char16_t(0x6C00), char16_t(0x6F00), char16_t(0x5D30), char16_t(0x8C30), char16_t(0x6F30), char16_t(0xC630), char16_t(0xB930), char16_t(0xC830), char16_t(0x6730), char16_t(0x5930)};
    auto u32hello = array{char32_t(0x00000068), char32_t(0x000000e9), char32_t(0x0000006c), char32_t(0x0000006c), char32_t(0x0000006f), char32_t(0x0000305d), char32_t(0x0000308c), char32_t(0x0000306f), char32_t(0x000030c6), char32_t(0x000030b9), char32_t(0x000030c8), char32_t(0x00003067), char32_t(0x00003059)};
    auto raw_hello = array{uint32_t(0x00000068), uint32_t(0x000000e9), uint32_t(0x0000006c), uint32_t(0x0000006c), uint32_t(0x0000006f), uint32_t(0x0000305d), uint32_t(0x0000308c), uint32_t(0x0000306f), uint32_t(0x000030c6), uint32_t(0x000030b9), uint32_t(0x000030c8), uint32_t(0x00003067), uint32_t(0x00003059)};

    auto from_u8 = hello | from<utf8>;
    auto from_u82 = u8hello | from<utf8>;
    auto from_u16le = u16lehello | from<utf16le>;
    auto from_u16be = u16behello | from<utf16be>;
    auto from_u32 = u32hello | from<utf32>;
    
    assert(cmp_iters(begin(from_u8), end(from_u8), begin(raw_hello), end(raw_hello)));
    assert(cmp_iters(begin(from_u82), end(from_u82), begin(raw_hello), end(raw_hello)));
    assert(cmp_iters(begin(from_u16be), end(from_u16be), begin(raw_hello), end(raw_hello)));
    assert(cmp_iters(begin(from_u16le), end(from_u16le), begin(raw_hello), end(raw_hello)));
    assert(cmp_iters(begin(from_u32), end(from_u32), begin(raw_hello), end(raw_hello)));

    {
        auto to_u8 = raw_hello | to<utf8>;
        auto to_u82 = raw_hello | to<utf8_u8>;
        auto to_u16be = raw_hello | to<utf16be>;
        auto to_u16le = raw_hello | to<utf16le>;
        auto to_u32 = raw_hello | to<utf32>;

        assert(cmp_iters(begin(hello), end(hello), begin(to_u8), end(to_u8)));
        assert(cmp_iters(begin(u8hello), end(u8hello), begin(to_u82), end(to_u82)));
        assert(cmp_iters(begin(u16behello), end(u16behello), begin(to_u16be), end(to_u16be)));
        assert(cmp_iters(begin(u16lehello), end(u16lehello), begin(to_u16le), end(to_u16le)));
        assert(cmp_iters(begin(u32hello), end(u32hello), begin(to_u32), end(to_u32)));
    }
    
    auto to_u8 = from_u8 | to<utf8>;
    auto to_u82 = from_u82 | to<utf8_u8>;
    auto to_u16be = from_u16be | to<utf16be>;
    auto to_u16le = from_u16le | to<utf16le>;
    auto to_u32 = from_u32 | to<utf32>;

    assert(cmp_iters(begin(hello), end(hello), begin(to_u8), end(to_u8)));
    assert(cmp_iters(begin(u8hello), end(u8hello), begin(to_u82), end(to_u82)));
    assert(cmp_iters(begin(u16behello), end(u16behello), begin(to_u16be), end(to_u16be)));
    assert(cmp_iters(begin(u16lehello), end(u16lehello), begin(to_u16le), end(to_u16le)));
    assert(cmp_iters(begin(u32hello), end(u32hello), begin(to_u32), end(to_u32)));

    chrono::nanoseconds dur_u8(0), dur_u16be(0), dur_u16le(0), dur_u32(0);
    constexpr long long total_iter = 100000;
    for (int i=0;i<total_iter;i++) {
        auto t0 = chrono::steady_clock::now();
        for (auto c : to_u8);
        auto t1 = chrono::steady_clock::now();
        for (auto c : to_u16be);
        auto t2 = chrono::steady_clock::now();
        for (auto c : to_u16le);
        auto t3 = chrono::steady_clock::now();
        for (auto c : to_u32);
        auto t4 = chrono::steady_clock::now();

        dur_u8+=t1-t0;
        dur_u16be+=t2-t1;
        dur_u16le+=t3-t2;
        dur_u32+=t4-t3;
    }
    std::cout << "All tests passed.\n";
    std::cout.imbue(std::locale("en_US.UTF-8"));
#ifdef __cpp_lib_format
    std::cout << "Bench on "<<total_iter<<" iters\n" 
        << format("utf8: total {}, per iter {}\n", dur_u8, dur_u8/total_iter)
        << format("utf16be: total {}, per iter {}\n", dur_u16be, dur_u16be/total_iter)
        << format("utf16le: total {}, per iter {}\n", dur_u16le, dur_u16le/total_iter)
        << format("utf32: total {}, per iter {}\n", dur_u32, dur_u32/total_iter);
#else
    std::cout << "Bench on "<<total_iter<<" iters\n" 
        << "utf8: total "<< dur_u8.count()<<"ns, per iter "<< dur_u8.count()/total_iter<<"ns\n"
        << "utf16be: total "<< dur_u16be.count()<<"ns, per iter "<< dur_u16be.count()/total_iter<<"ns\n"
        << "utf16le: total "<< dur_u16le.count()<<"ns, per iter "<< dur_u16le.count()/total_iter<<"ns\n"
        << "utf32: total "<< dur_u32.count()<<"ns, per iter "<< dur_u32.count()/total_iter<<"ns\n";
#endif
    std::cout << "Display : ";
    auto from_u16_to_u8 = u"hélloそれはテストです" | from<utf16be> | to<utf8>;
    for (auto c : from_u16_to_u8)
        std::cout.put(c);
    std::cout.put('\n');
}
int main()
{
    using namespace rpp::conv;
    using namespace std::literals;
    
    tests();
    return 0;
}