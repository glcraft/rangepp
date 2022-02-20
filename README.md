# Range++

A library to extend the \<ranges\> STL library.

# Required

C++20 concepts and ranges are reuqired to compile with the library.

# Ranges modules

## `conv` - Character encode/decode

Conversion from and to different character encoding. Actually only unicode encodings are implemented in the library, but it can be extended.

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

    // Classic range usage
    auto str = u"hélloそれはテストです"s;
    auto rng_utf8 = str | from<utf16be> | to<utf8_u8>;
    auto utf8str = std::u8string(rng_utf8.begin(), rng_utf8.end());
    assert(utf8str == std::u8string_view(u8"hélloそれはテストです"));

    // Using rpp::conv::convert helper
    auto rng_u16_u8 = str | convert<utf16be, utf8_u8>;

    // Collect directly into string
    auto string_u16_u8 = str | convert<utf16be, utf8_u8> | rpp::collect<std::u8string>;
    return 0;
}
```

## `math` - Mathematics manipulation (WIP)

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