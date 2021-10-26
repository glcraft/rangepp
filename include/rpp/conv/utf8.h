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

        template <class Converter, typename CharType>
        concept is_char_converter = requires (CharType arg_length, std::array<CharType, Converter::max_char> arg_from, uint32_t arg_to){
            std::same_as<typename Converter::char_type, CharType>;
            std::same_as<decltype(Converter::max_char), uint64_t>;
            {Converter::length(arg_length)}->std::same_as<uint32_t>;
            {Converter::from(arg_from)} -> std::convertible_to<uint32_t>;
            {Converter::to(arg_to)} -> std::convertible_to<CharsInfo<CharType, Converter::max_char>>;
    
        };
        
        template <typename CharType, is_char_converter<CharType> Converter>
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
                
                from() : m_iter(nullptr)
                {}
                from(Iterator&& it) : m_iter(std::move(it))
                {}

                [[nodiscard]] constexpr decltype(auto) operator*() const noexcept {
                    std::array<CharType, Converter::max_char> seq;
                    auto it = m_iter;
                    seq[0] = *it++;
                    auto len = Converter::length(seq[0]);
                    for(decltype(len) i=1;i<len;++i)
                        seq[i] = *it++;
                    return Converter::from(seq);
                }
                constexpr from& operator++() 
                {
                    std::advance(m_iter, Converter::length(*m_iter));
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
            template <std::ranges::viewable_range InputRange>
                requires std::forward_iterator<std::ranges::iterator_t<InputRange>>
            class to
            {
                using Base = const InputRange;
                using Iterator = std::ranges::iterator_t<InputRange>;
                Iterator m_iter;
                CharsInfo<CharType, Converter::max_char> mutable m_chars;
                char mutable m_current_char = Converter::max_char;
            public:
                using input_type = uint32_t;
                using output_type = CharType;

                using iterator_concept = rpp::impl::tag_from_range_t<InputRange>;
                using value_type = output_type;
                using difference_type = std::ranges::range_difference_t<Base>;

                to() : m_iter(nullptr)
                {}
                to(Iterator&& it) : m_iter(std::move(it))
                {}
                [[nodiscard]] constexpr decltype(auto) operator*() const {
                    if (m_current_char >= m_chars.size)
                        read_char();
                    return m_chars.chars[m_current_char];
                }
                constexpr to& operator++() {
                    ++m_current_char;
                    if (m_current_char >= m_chars.size)
                    {
                        ++m_iter;
                        m_current_char = Converter::max_char;
                    }
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
            private:
                constexpr void read_char() const {
                    m_chars = Converter::to(*m_iter);
                    m_current_char = 0;
                }
            };
        };
        struct Utf8Conv {
            static constexpr uint64_t max_char = 4;
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
            static constexpr uint64_t max_char = 2;
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