#pragma once
#include <optional>
#include <array>
namespace rpp
{
    namespace conv
    {
        template <typename Ch>
        concept utf8_char = requires {
            sizeof(std::remove_reference_t<Ch>) == 1;
        };
        class utf8_converter
        {
            std::array<char8_t, 4> m_chars = {};
            char m_current=4, m_size=0;
        public:
            using input_type = char8_t;
            using output_type = char8_t;
            constexpr utf8_converter() = default;
            // constexpr utf8_converter(const utf8_converter&) = default;
            // constexpr utf8_converter(utf8_converter&&) = default;

            template <typename Iter>
                requires std::forward_iterator<Iter> && requires (Iter a){//utf8_char<typename Iter::value_type>
                    {sizeof(*a)==1};
                }
            [[nodiscard]] constexpr uint32_t from(const Iter& iter) const noexcept {
                auto myiter = iter;
                auto fschar = static_cast<uint8_t>(*(myiter++));
                auto nchar = GetCharBytes(fschar);
                {
                    if (nchar == 1)
                        return fschar;
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
            template <typename Iter>
                requires std::forward_iterator<Iter> && requires (Iter a){//utf8_char<typename Iter::value_type>
                    {*a} -> std::convertible_to<uint32_t>;
                }
            [[nodiscard]] constexpr output_type to(const Iter& iter) noexcept {
                
                if (m_current >= m_size)
                {
                    constexpr auto create_subchar = [](auto ch, unsigned char add = 0x80) {
                        return  static_cast<char>(add | static_cast<unsigned char>(ch));
                    };
                    auto ch = *iter;
                    if (ch <= 0x7F)
                    {
                        m_chars[0] = ch;
                        m_size = 1;
                    }
                    else if (ch <= 0x07FF)
                    {
                        m_chars[0] = create_subchar(ch >> 6, 0xC0);
                        m_chars[1] = create_subchar(ch & 0x3f);
                        m_size = 2;
                    }
                    else if (ch <= 0xFFFF)
                    {
                        m_chars[0] = create_subchar(ch >> 12, 0xE0);
                        m_chars[1] = create_subchar((ch >> 6) & 0x3f);
                        m_chars[2] = create_subchar(ch & 0x3f);
                        m_size = 3;
                    }
                    else
                    {
                        m_chars[0] = create_subchar(ch >> 18, 0xE0);
                        m_chars[1] = create_subchar((ch >> 12) & 0x3f);
                        m_chars[2] = create_subchar((ch >> 6) & 0x3f);
                        m_chars[3] = create_subchar(ch & 0x3f);
                        m_size = 4;
                    }
                    m_current=0;
                }
                return m_chars[m_current];
            }
            constexpr int remains_characters() const {
                return m_size - m_current;
            }
            constexpr void next()
            {
                ++m_current;
            }
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
        private:
            
        };
        inline constexpr utf8_converter utf8 = {};
    }
}