#pragma once
#include <array>
#include <concepts>
#include "char_conv.h"
namespace rpp
{
    namespace conv
    {
        // template <typename Ch>
        // concept one_byte_char = requires {
        //     sizeof(std::remove_reference_t<Ch>) == 1;
        // };
        template <typename Iter>
        concept utf8_from_iterator = std::forward_iterator<Iter> 
            && std::copyable<Iter>;
        template <typename Iter>
        concept utf8_to_iterator = std::forward_iterator<Iter> 
            && std::copyable<Iter>;
        
        struct Utf8Conv {
            static constexpr uint32_t max_char = 4;
            using char_type = char;
            using sequence_type = std::array<char_type, max_char>;

            static constexpr uint32_t length(char_type ch) {
                if ((ch & 0x80) == 0)
                    return 1;
                else if ((ch & 0xE0) == 0xC0)
                    return 2;
                else if ((ch & 0xF0) == 0xE0)
                    return 3;
                else if ((ch & 0xF8) == 0xF0)
                    return 4;
                else
                    return 1;
            }
            static constexpr uint32_t from(std::array<char_type, max_char> ch) {
                auto len = length(ch[0]);
                if (len == 1)
                    return static_cast<uint32_t>(ch[0]);
                else
                {
                    uint32_t result = static_cast<uint32_t>(ch[0] & (0x7F >> len)) << 6;
                    auto it=std::data(ch)+1;
                    auto end = std::data(ch)+len;
                    for (; it!=end; result<<6, ++it) {
                        result += static_cast<uint32_t>((*it) & 0x3F);
                    }
                    return result;
                }
            }
            static constexpr CharsInfo<char_type, max_char> to(uint32_t ch) {
                constexpr auto create_subchar = [](auto ch, unsigned char add = 0x80) {
                    return  static_cast<char>(add | static_cast<unsigned char>(ch));
                };
                CharsInfo<char, 4> result;
                
                if (ch <= 0x7F)
                {
                    result.chars[0] = ch;
                    result.size = 1;
                }
                else if (ch <= 0x07FF)
                {
                    result.chars[0] = create_subchar(ch >> 6, 0xC0);
                    result.chars[1] = create_subchar(ch & 0x3f);
                    result.size = 2;
                }
                else if (ch <= 0xFFFF)
                {
                    result.chars[0] = create_subchar(ch >> 12, 0xE0);
                    result.chars[1] = create_subchar((ch >> 6) & 0x3f);
                    result.chars[2] = create_subchar(ch & 0x3f);
                    result.size = 3;
                }
                else
                {
                    result.chars[0] = create_subchar(ch >> 18, 0xE0);
                    result.chars[1] = create_subchar((ch >> 12) & 0x3f);
                    result.chars[2] = create_subchar((ch >> 6) & 0x3f);
                    result.chars[3] = create_subchar(ch & 0x3f);
                    result.size = 4;
                }
                return result;
            }
        };
        template <bool BigEndian>
        struct Utf16Conv {
            static constexpr uint32_t max_char = 2;
            static constexpr bool is_big_endian = BigEndian;
            using char_type = char16_t;
            
            static constexpr char16_t swap_endian(char16_t ch) {
                if constexpr(is_big_endian)
                    return ch;
                else
                    return (ch << 8) + (ch >> 8);
            }

            static constexpr uint32_t length(char_type ch) {
                return 1 + uint32_t((swap_endian(ch)&0xFC00) == 0xD800);
            }
            static constexpr uint32_t from(std::array<char_type, max_char> ch) {
                auto len = length(ch[0]);
                for (auto& c : ch)
                    c =swap_endian(c);
                if ((ch[0] & 0xFC00) == 0xD800)
                {
                    auto w = static_cast<uint32_t>(((ch[0] & 0x3C0)>>6)+1);
                    auto x = static_cast<uint32_t>(ch[0] & 0x3F);
                    auto y = static_cast<uint32_t>(ch[1] & 0x3FF);
                    return (w << 16) + (x << 10) + y;
                }
                else
                    return static_cast<uint32_t>(ch[0]);
            }
            static constexpr CharsInfo<char_type, max_char> to(char_type ch) {
                CharsInfo<char16_t, 2> result;
                if (ch <= 0xFFFF)
                {
                    result.chars[0] = static_cast<char16_t>(ch);
                    result.size=1;
                }
                else
                {
                    result.chars[0] = static_cast<char16_t>(0xD800 | ((ch >> 10) & 0x3FF + (1 << 6)));
                    result.chars[1] = static_cast<char16_t>(0xDC00 | (ch & 0x3FF));
                    result.size=2;
                }
                for (auto& c : result.chars)
                    c = swap_endian(c);
                return result;
            }
        };
        using utf8 = char_converter<char, Utf8Conv>;
        using utf16be = char_converter<char16_t, Utf16Conv<true>>;
        using utf16le = char_converter<char16_t, Utf16Conv<false>>;
        // using u8_utf8 = char_converter<char8_t, [](auto it) { return it; }, [](auto it) { return it; }>;
    }
}