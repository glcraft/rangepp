#pragma once
#include <optional>
#include <array>
#include <concepts>
#include "../conv.h"
namespace rpp
{
    namespace conv
    {
        template <typename Ch>
        concept one_byte_char = requires {
            sizeof(std::remove_reference_t<Ch>) == 1;
        };
        template <typename Iter>
        concept utf8_from_iterator = std::forward_iterator<Iter> 
            && std::copyable<Iter>
            && requires (Iter a) {
                {*a} -> one_byte_char;
            };
        template <typename Iter>
        concept utf8_to_iterator = std::forward_iterator<Iter> 
            && std::copyable<Iter>
            && requires (Iter a) {
                {*a} -> std::convertible_to<uint32_t>;
            };
        
        template <one_byte_char CharType>
        class utf8_converter
        {
        public:
            using input_type = CharType;
            using output_type = CharType;
            constexpr utf8_converter() = default;
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
                    auto myiter = m_iter;
                    auto fschar = static_cast<uint8_t>(*(myiter++));
                    auto nchar = GetCharBytes(fschar);
                    {
                        if (nchar == 1)
                            return static_cast<uint32_t>(fschar);
                        else
                        {
                            uint32_t result = 0;
                            result = static_cast<uint32_t>(fschar & (0x7F >> nchar)) << 6;
                            for (int i = nchar - 1; i > 0; i--, result<<6, ++myiter)
                            {
                                auto test = static_cast<uint8_t>(*myiter);
                                result += static_cast<uint32_t>(test & 0x3F);
                            }
                            return result;
                        }
                    }
                }
                constexpr from& operator++() 
                {
                    std::advance(m_iter, utf8_converter::GetCharBytes(*m_iter));
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
                struct CharsInfo
                {
                    std::array<CharType, 4> chars = {};
                    char current=4, size=0;
                    constexpr void load_char(uint32_t ch) noexcept
                    {
                        constexpr auto create_subchar = [](auto ch, unsigned char add = 0x80) {
                            return  static_cast<char>(add | static_cast<unsigned char>(ch));
                        };
                        
                        if (ch <= 0x7F)
                        {
                            chars[0] = ch;
                            size = 1;
                        }
                        else if (ch <= 0x07FF)
                        {
                            chars[0] = create_subchar(ch >> 6, 0xC0);
                            chars[1] = create_subchar(ch & 0x3f);
                            size = 2;
                        }
                        else if (ch <= 0xFFFF)
                        {
                            chars[0] = create_subchar(ch >> 12, 0xE0);
                            chars[1] = create_subchar((ch >> 6) & 0x3f);
                            chars[2] = create_subchar(ch & 0x3f);
                            size = 3;
                        }
                        else
                        {
                            chars[0] = create_subchar(ch >> 18, 0xE0);
                            chars[1] = create_subchar((ch >> 12) & 0x3f);
                            chars[2] = create_subchar((ch >> 6) & 0x3f);
                            chars[3] = create_subchar(ch & 0x3f);
                            size = 4;
                        }
                        current=0;
                    }
                } mutable m_chars;
                
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
                    if (m_chars.current >= m_chars.size)
                        m_chars.load_char(*m_iter);
                    return m_chars.chars[m_chars.current];
                }
                constexpr to& operator++() {
                    ++m_chars.current;
                    if (m_chars.current >= m_chars.size)
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
            static int GetCharBytes(uint8_t ch) {
                if ((ch & 0x80) == 0)
                    return 1;
                if ((ch & 0xE0) == 0xC0)
                    return 2;
                if ((ch & 0xF0) == 0xE0)
                    return 3;
                if ((ch & 0xF8) == 0xF0)
                    return 4;
                return 1; // default : char copy and advance
            }
        };
        using utf8 = utf8_converter<char>;
        using u8_utf8 = utf8_converter<char8_t>;
    }
}