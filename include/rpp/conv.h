#pragma once
#include "common.h"
#include <ranges>
#include <concepts>
#include "conv/utf8.h"

namespace rpp
{
    namespace conv
    {
        template <typename T, typename Iter>
        concept from_converter = requires (T a) {
            typename T::input_type;
        } 
        && requires (T a, Iter& it){
            {a.from(it)} -> std::convertible_to<uint32_t>;
        }
        && requires (T a, typename T::input_type v){
            {T::GetCharBytes(v)}-> std::convertible_to<int>;
        };
        template <typename T>
        concept to_converter = requires (T a, uint32_t i) {
            typename T::output_type;
            {a.to(i)} -> std::convertible_to<typename T::output_type>;
        };
        template <typename T, typename Iter>
        concept full_converter = from_converter<T, Iter> && to_converter<T>;

        namespace impl
        {       
            template <std::ranges::forward_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            class from_view : public std::ranges::view_interface<from_view<Range, Converter>>{
                Range m_rng;
                Converter m_conv;
            public:
                class iterator {
                    using Self = iterator;
                    using Parent = from_view<Range, Converter>;
                    using Base = Range;
                    std::ranges::iterator_t<Base> m_current;
                    std::reference_wrapper<Parent> m_parent;
                public:
                    using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                    // using iterator_category = ;
                    using value_type = uint32_t;
                    using difference_type = std::ranges::range_difference_t<Base>;

                    iterator() requires std::default_initializable<std::ranges::iterator_t<Base>> = default;
                    iterator(std::ranges::iterator_t<Base> current, Parent& parent) : m_current(std::move(current)) , m_parent(parent)
                    {}
                    [[nodiscard]] constexpr value_type operator*()
                    {
                        return m_parent.get().m_conv.template from<std::ranges::iterator_t<Base>>(m_current);
                    }
                    constexpr Self &operator++()
                    {
                        std::advance(m_current, Converter::GetCharBytes(*m_current));
                        return *this;
                    }
                    constexpr Self &operator++(int)
                    {
                        auto tmp = *this;
                        std::advance(m_current, Converter::GetCharBytes(*m_current));
                        return tmp;
                    }
                    constexpr Self &operator+=(difference_type n) requires std::ranges::random_access_range<Base>
                    {
                        m_current += n;
                        return *this;
                    }
                    [[nodiscard]] friend constexpr bool operator==(const Self &x, const Self &y)
                    {
                        return x.m_current == y.m_current;
                    }
                };
                from_view() requires std::default_initializable<Range>
                {}
                constexpr from_view(Range rng, Converter conv) : m_rng(std::move(rng)), m_conv(conv)
                {}

                [[nodiscard]] inline constexpr Range base() const& noexcept {
                    return m_rng;
                }
                [[nodiscard]] inline constexpr Range base() && noexcept {
                    return std::move(m_rng);
                }
                constexpr auto begin() const
                {
                    return iterator{std::ranges::begin(m_rng)};
                }
                constexpr auto begin()
                {
                    return iterator{std::ranges::begin(m_rng), *this};
                }
                constexpr auto end() const
                {
                    return iterator{std::ranges::end(m_rng)};
                }
                constexpr auto end()
                {
                    return iterator{std::ranges::end(m_rng), *this};
                }
            };
            template <typename T>
            struct from_store {
                T c;
            };
            class from_fn
            {
                public:
                    template <std::ranges::forward_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
                    inline constexpr auto operator()(Range rng, Converter conv) const noexcept
                    {
                        return from_view(std::move(rng), std::move(conv));
                    }
                    template <typename T>
                    inline constexpr auto operator()(T conv) const noexcept
                    {
                        return from_store{std::move(conv)};
                    }
                private:
            };
            template <std::ranges::forward_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            from_view<Range, Converter> operator|(Range rng, from_store<Converter> store)
            {
                return from_view(std::move(rng), std::move(store.c));
            }
        } // namespace impl
        inline constexpr impl::from_fn from;
    } // namespace conv
}