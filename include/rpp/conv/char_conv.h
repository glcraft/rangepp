#pragma once
#include <concepts>
#include <array>
namespace rpp
{
    namespace conv
    {
        template <class Converter, typename CharType>
        concept is_char_converter = requires (CharType arg_length, std::array<CharType, Converter::max_char+1> arg_from, uint32_t arg_to){
            std::same_as<typename Converter::char_type, CharType>;
            std::same_as<decltype(Converter::max_char), uint32_t>;
            {Converter::length(arg_length)}->std::same_as<uint32_t>;
            {Converter::from(arg_from)} -> std::convertible_to<uint32_t>;
            {Converter::to(arg_to)} -> std::convertible_to<std::array<CharType, Converter::max_char+1>>;
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
                    std::array<CharType, Converter::max_char+1> seq;
                    auto it = m_iter;
                    seq[0] = *it++;
                    auto len = Converter::length(seq[0]);
                    for(decltype(len) i=1;i<len;++i)
                        seq[i] = *it++;
                    seq[len] = '\0';
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
                std::array<CharType, Converter::max_char+1> mutable m_chars;
                char m_current_char = 0;
            public:
                using input_type = uint32_t;
                using output_type = CharType;

                using iterator_concept = rpp::impl::tag_from_range_t<InputRange>;
                using value_type = output_type;
                using difference_type = std::ranges::range_difference_t<Base>;

                to() : m_iter(nullptr)
                {}
                to(Iterator&& it) : m_iter(std::move(it))
                {
                    for (auto& c : m_chars)
                        c = '\0';
                }
                [[nodiscard]] constexpr decltype(auto) operator*() const {
                    auto test = *m_iter;
                    if (current_char() == 0)
                        read_char();
                    return current_char();
                }
                constexpr to& operator++() {
                    ++m_current_char;
                    if (current_char() == 0)
                    {
                        ++m_iter;
                        read_char();
                        m_current_char = 0;
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
                    return 
                        x.m_iter == y.m_iter
                        && x.m_current_char == y.m_current_char;
                }
            private:
                constexpr CharType current_char() const noexcept {
                    return m_chars[m_current_char];
                }
                constexpr void read_char() const {
                    auto c = *m_iter;
                    m_chars = Converter::to(c);
                }
            };
        };
    }
}