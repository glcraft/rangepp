#include <concepts>
#include <array>
namespace rpp
{
    namespace conv
    {
        template <typename CharType, size_t CharNumb>
        struct CharsInfo
        {
            std::array<CharType, CharNumb> chars = {};
            char size=0;
        };

        template <class Converter, typename CharType>
        concept is_char_converter = requires (CharType arg_length, std::array<CharType, Converter::max_char> arg_from, uint32_t arg_to){
            std::same_as<typename Converter::char_type, CharType>;
            std::same_as<decltype(Converter::max_char), uint32_t>;
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
    }
}