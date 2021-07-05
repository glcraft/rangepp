#pragma once
#include "common.h"
#include <ranges>
#include <concepts>

namespace rpp
{
    namespace conv
    {
        template <typename T>
        concept from_converter = requires (T a) {
            typename T::input_type;
            {a.from(T::input_t())} -> std::convertible_to<uint32_t>;
        };
        template <typename T>
        concept to_converter = requires (T a, uint32_t i) {
            typename T::output_type;
            {a.to(i)} -> std::convertible_to<typename T::output_type>;
        };
        template <typename T>
        concept full_converter = from_converter<T> && to_converter<T>;

        namespace impl
        {       
            template <std::ranges::input_range Range>
            class from_view : public std::ranges::view_interface<from_view<Range>>{
                Range m_rng;
            public:
                class iterator {
                    using Self = iterator;
                    using Parent = from_view<Range>;
                    using Base = Range;
                    std::ranges::iterator_t<Base> m_current;
                public:
                    using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                    // using iterator_category = ;
                    using value_type = typename Base::value_type;
                    using difference_type = std::ranges::range_difference_t<Base>;

                    iterator() requires std::default_initializable<iterator_t<Base>> = default;
                    iterator(std::ranges::iterator_t<Base> current) : m_current(std::move(current)) 
                    {}
                    constexpr decltype(auto) operator*() const
                    {
                        return *m_current + 1;
                    }
                    constexpr Self &operator++()
                    {
                        ++m_current;
                        return *this;
                    }
                    constexpr void operator++(int)
                    {
                        ++m_current;
                    }
                    constexpr Self &operator++(int) requires forward_range<Base>
                    {
                        auto tmp = *this;
                        ++*this;
                        return tmp;
                    }
                    constexpr Self &operator--()
                    {
                        --m_current;
                        return this;
                    }
                    constexpr Self &operator--(int) requires forward_range<Base>
                    {
                        auto tmp = *this;
                        --*this;
                        return tmp;
                    }
                    constexpr Self &operator+=(difference_type n) requires random_access_range<Base>
                    {
                        m_current += n;
                        return *this;
                    }
                    constexpr Self &operator-=(difference_type n) requires random_access_range<Base>
                    {
                        m_current -= n;
                        return *this;
                    }
                    [[nodiscard]] friend constexpr bool operator==(const Self &x, const Self &y)
                    {
                        return x.m_current == y.m_current;
                    }
                };
                from_view() requires std::default_initializable<Range>
                {}
                constexpr from_view(Range rng) : m_rng(std::move(rng))
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
                    return iterator{std::ranges::begin(m_rng)};
                }
                constexpr auto end() const
                {
                    return iterator{std::ranges::end(m_rng)};
                }
                constexpr auto end()
                {
                    return iterator{std::ranges::end(m_rng)};
                }
            };
            
            class from_fn
            {
                public:
                    template <std::ranges::input_range Range>
                    inline constexpr auto operator()(Range&& rng) const noexcept
                    {
                        return from_view(std::move(rng));
                    }
                private:
            };
            template <std::ranges::input_range Range>
            from_view<Range> operator|(Range rng, from_fn)
            {
                return from_view(std::move(rng));
            }
        } // namespace impl
        inline constexpr impl::from_fn from;
    } // namespace conv
}