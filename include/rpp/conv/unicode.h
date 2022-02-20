#pragma once
#include <array>
#include <concepts>
#include "char_conv.h"
namespace rpp
{
    namespace conv
    {
        template <typename CharType>
        struct Utf8Conv {
            static constexpr uint32_t max_char = 4;
            using char_type = CharType;
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
            static constexpr uint32_t from(std::array<char_type, max_char+1> ch) {
                auto len = length(ch[0]);
                if (len == 1)
                    return static_cast<uint32_t>(ch[0]);
                else
                {
                    uint32_t result = static_cast<uint32_t>(ch[0] & (0x7F >> len));
                    auto it=std::data(ch)+1;
                    auto end = std::data(ch)+len;
                    do {
                        result<<=6;
                        result += static_cast<uint32_t>((*it) & 0x3F);
                    } while (++it != end);
                    return result;
                }
            }
            static constexpr std::array<char_type, max_char+1> to(uint32_t ch) {
                constexpr auto create_subchar = [](auto ch, unsigned char add = 0x80) {
                    return  static_cast<CharType>(add | static_cast<unsigned char>(ch));
                };
                std::array<char_type, 5> result;
                
                if (ch <= 0x7F)
                {
                    result[0] = ch;
                    result[1] = '\0';
                }
                else if (ch <= 0x07FF)
                {
                    result[0] = create_subchar(ch >> 6, 0xC0);
                    result[1] = create_subchar(ch & 0x3f);
                    result[2] = '\0';
                }
                else if (ch <= 0xFFFF)
                {
                    result[0] = create_subchar(ch >> 12, 0xE0);
                    result[1] = create_subchar((ch >> 6) & 0x3f);
                    result[2] = create_subchar(ch & 0x3f);
                    result[3] = '\0';
                }
                else
                {
                    result[0] = create_subchar(ch >> 18, 0xE0);
                    result[1] = create_subchar((ch >> 12) & 0x3f);
                    result[2] = create_subchar((ch >> 6) & 0x3f);
                    result[3] = create_subchar(ch & 0x3f);
                    result[4] = '\0';
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
            static constexpr uint32_t from(std::array<char_type, max_char+1> ch) {
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
            static constexpr std::array<char_type, max_char+1> to(char_type ch) {
                std::array<char16_t, 3> result;
                if (ch <= 0xFFFF)
                {
                    result[0] = static_cast<char16_t>(ch);
                    result[1] = '\0';
                }
                else
                {
                    result[0] = static_cast<char16_t>(0xD800 | ((ch >> 10) & 0x3FF + (1 << 6)));
                    result[1] = static_cast<char16_t>(0xDC00 | (ch & 0x3FF));
                    result[2] = '\0';
                }
                for (auto& c : result)
                    c = swap_endian(c);
                return result;
            }
        };
        struct Utf32Conv {
            static constexpr uint32_t max_char = 1;
            using char_type = char32_t;
            static constexpr uint32_t length(char_type ch) {
                return 1;
            }
            static constexpr uint32_t from(std::array<char_type, max_char+1> ch) {
                return ch[0];
            }
            static constexpr std::array<char_type, max_char+1> to(char_type ch) {
                return {ch, '\0'};
            }
        };
        using utf8 = char_converter<char, Utf8Conv<char>>;
        using utf8_u8 = char_converter<char8_t, Utf8Conv<char8_t>>;
        using utf16be = char_converter<char16_t, Utf16Conv<true>>;
        using utf16le = char_converter<char16_t, Utf16Conv<false>>;
        using utf32 = char_converter<char32_t, Utf32Conv>;
    }
}