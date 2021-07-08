#pragma once
#include <optional>
#include <array>
#include <concepts>
#include "../conv.h"
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
        template <typename CharType, size_t CharNumb>
        struct CharsInfo
        {
            std::array<CharType, CharNumb> chars = {};
            char size=0;
        };
        
        template <typename CharType, size_t CharNumb, auto advance_fn, auto from_func, auto to_func>
        class char_converter
        {
        public:
            using input_type = CharType;
            using output_type = CharType;
            constexpr char_converter() = default;
            template <std::ranges::viewable_range Range>
                requires std::forward_iterator<std::ranges::iterator_t<Range>>
            class from
            {
                using Base = const Range;
                using Iterator = std::ranges::iterator_t<Range>;
                Iterator m_iter;
            public:
                using input_type = CharType;
                using output_type = uint32_t;

                using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                using value_type = output_type;
                using difference_type = std::ranges::range_difference_t<Base>;
                
                from()
                {}
                from(Iterator&& it) : m_iter(std::move(it))
                {}

                [[nodiscard]] constexpr decltype(auto) operator*() const noexcept {
                    return from_func(m_iter);
                }
                constexpr from& operator++() 
                {
                    // std::advance(m_iter, char_converter::GetCharBytes(*m_iter));
                    advance_fn(m_iter);
                    return *this;
                }
                constexpr from operator++(int) 
                {
                    auto tmp = *this;
                    ++*this;
                    return tmp;
                }
                [[nodiscard]] friend constexpr bool operator==(const from &x, const from &y)
                {
                    return x.m_iter == y.m_iter;
                }
            };
            template <std::ranges::viewable_range Range>
                requires std::forward_iterator<std::ranges::iterator_t<Range>>
            class to
            {
                using Base = const Range;
                using Iterator = std::ranges::iterator_t<Range>;
                Iterator m_iter;
                CharsInfo<CharType, CharNumb> mutable m_chars;
                char mutable m_current_char = CharNumb;
            public:
                using input_type = uint32_t;
                using output_type = CharType;

                using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                using value_type = output_type;
                using difference_type = std::ranges::range_difference_t<Base>;

                to() : m_iter(nullptr)
                {}
                to(Iterator&& it) : m_iter(std::move(it))
                {}
                [[nodiscard]] constexpr CharType operator*() const noexcept {
                    if (m_current_char >= m_chars.size)
                    {
                        m_chars = to_func(m_iter);
                        m_current_char = 0;
                    }
                    return m_chars.chars[m_current_char];
                }
                constexpr to& operator++() {
                    ++m_current_char;
                    if (m_current_char >= m_chars.size)
                        ++m_iter;
                    return *this;
                }
                constexpr to operator++(int) 
                {
                    auto tmp = *this;
                    ++*this;
                    return tmp;
                }
                [[nodiscard]] friend constexpr bool operator==(const to &x, const to &y)
                {
                    return x.m_iter == y.m_iter;
                }
                
            };
        };
        using utf8 = char_converter<char, 4, 
            [](auto& it) {
                auto ch = *it;
                if ((ch & 0x80) == 0)
                    std::advance(it, 1);
                else if ((ch & 0xE0) == 0xC0)
                    std::advance(it, 2);
                else if ((ch & 0xF0) == 0xE0)
                    std::advance(it, 3);
                else if ((ch & 0xF8) == 0xF0)
                    std::advance(it, 4);
                else
                    std::advance(it, 1);
            },
            [](auto myiter) { 
                auto fschar = static_cast<uint8_t>(*(myiter++));
                auto nchar = [](uint8_t ch) {
                    if ((ch & 0x80) == 0)
                        return 1;
                    if ((ch & 0xE0) == 0xC0)
                        return 2;
                    if ((ch & 0xF0) == 0xE0)
                        return 3;
                    if ((ch & 0xF8) == 0xF0)
                        return 4;
                    return 1; // default : char copy and advance
                }(fschar);
                {
                    if (nchar == 1)
                        return static_cast<uint32_t>(fschar);
                    else
                    {
                        uint32_t result = 0;
                        result = static_cast<uint32_t>(fschar & (0x7F >> nchar)) << 6;
                        for (int i = nchar - 1; i != 0; i--, result<<6, ++myiter)
                        {
                            auto test = static_cast<uint8_t>(*myiter);
                            result += static_cast<uint32_t>(test & 0x3F);
                        }
                        return result;
                    }
                }
            }, 
            [](const auto& it) { 
                constexpr auto create_subchar = [](auto ch, unsigned char add = 0x80) {
                    return  static_cast<char>(add | static_cast<unsigned char>(ch));
                };
                CharsInfo<char, 4> result;
                auto ch = *it;
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
            }>;
        using utf16be = char_converter<char16_t, 2, 
            [](auto &it) {
                auto ch = *it;
                std::advance(it, 1+int((ch&0xFC00) == 0xD800));    
            },
            [](auto it) -> uint32_t{
                
                auto fsch = *it;
                if ((fsch & 0xFC00) == 0xD800)
                {
                    char16_t ch = *std::next(it);
                    uint32_t w = ((fsch & 0x3C0)>>6)+1;
                    uint32_t x = fsch & 0x3F;
                    uint32_t y = ch & 0x3FF;
                    return (w << 16) + (x << 10) + y;
                }
                else
                    return fsch;
            },
            [](const auto &it) {
                auto ch = *it;
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
                return result;
            }
        >;
        constexpr char16_t swap_endian(char16_t ch)
        {
            return (ch << 8) + (ch >> 8);
        }
        using utf16le = char_converter<char16_t, 2, 
            [](auto &it) {
                auto ch = *it;
                std::advance(it, 1+int((ch&0xFC00) == 0xD800));    
            },
            [](auto it) -> uint32_t{
                
                auto fsch = swap_endian(*it);
                if ((fsch & 0xFC00) == 0xD800)
                {
                    char16_t ch = swap_endian(*std::next(it));
                    uint32_t w = ((fsch & 0x3C0)>>6)+1;
                    uint32_t x = fsch & 0x3F;
                    uint32_t y = ch & 0x3FF;
                    return (w << 16) + (x << 10) + y;
                }
                else
                    return fsch;
            },
            [](const auto &it) {
                auto ch = *it;
                CharsInfo<char16_t, 2> result;
                if (ch <= 0xFFFF)
                {
                    result.chars[0] = swap_endian(ch);
                    result.size=1;
                }
                else
                {
                    result.chars[0] = swap_endian(0xD800 | ((ch >> 10) & 0x3FF + (1 << 6)));
                    result.chars[1] = swap_endian(0xDC00 | (ch & 0x3FF));
                    result.size=2;
                }
                return result;
            }
        >;
        // using u8_utf8 = char_converter<char8_t, [](auto it) { return it; }, [](auto it) { return it; }>;
    }
}