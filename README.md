# Range++

A library to extend the \<ranges\> STL library.

# Required

C++20 concepts and ranges are reuqired to compile with the library.

# Ranges modules

## `conv` - Character encode/decode

Conversion from and to different character encoding. Actually only unicode encodings are implemented.

Available: 
* utf8 (for `char`)
* utf8_u8 (for `char8_t`)
* utf16be
* utf16le
* utf32

## Example

```cpp
int main()
{
    using namespace rpp::conv;
    auto str = std::views::all(u"你好"); // Hello
    auto rng_utf8 = str | from<utf16be> | to<utf8_u8>;
    auto utf8str = std::u8string(rng_utf8.begin(), rng_utf8.end());
    assert(utf8str == std::u8string_view(u8"你好"));
    return 0;
}
```

## `math` - Mathematics manipulation

Work in progress. Make operation on ranges values.

Available: 
* add_const
* minus_const
* mult_const
* div_const
* mod_const
* bit_and_const
* bit_or_const
* bit_xor_const

## Example

```cpp
int main()
{
    using namespace rpp::conv;
    auto ls = std::array{0.f,1.f,2.f,3.f};
    auto incr_ls = ls | rpp::math::add_const<0.5f>;
    for (auto c : incr_ls)
        std::cout << c << ' ';
    return 0;
}
// Result: 0.5 1.5 2.5 3.5
```