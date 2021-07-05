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
            std::optional<std::array<char8_t, 4>> m_chars;
        public:
            using input_type = char8_t;
            constexpr utf8_converter() = default;
            constexpr utf8_converter(const utf8_converter&) = default;
            constexpr utf8_converter(utf8_converter&&) = default;

            template <typename Iter>
                requires std::forward_iterator<Iter> && utf8_char<typename Iter::value_type>
            [[nodiscard]] constexpr uint32_t from(const Iter& iter) noexcept {
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
        inline constexpr utf8_converter utf8;
    }
}