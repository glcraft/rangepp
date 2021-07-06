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
        template <typename T, typename Iter>
        concept to_converter = requires (T a) {
            typename T::output_type;
            {a.remains_characters()} -> std::convertible_to<int>;
            {a.next()};
        }
        && requires (T a, const Iter& it){
            {*it} -> std::convertible_to<uint32_t>;
            {a.to(it)} -> std::convertible_to<typename T::output_type>;
        };
        template <typename T, typename Iter>
        concept full_converter = from_converter<T, Iter> && to_converter<T, Iter>;

        namespace impl
        {       
            template <std::ranges::viewable_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            class from_view : public std::ranges::view_interface<from_view<Range, Converter>>{
                Range m_rng;
                Converter m_conv;
            public:
                class iterator {
                    using Self = iterator;
                    using Parent = from_view<Range, Converter>;
                    using Base = const Range;
                    std::ranges::iterator_t<Base> m_current;
                    Converter m_conv;
                public:
                    using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                    using value_type = uint32_t;
                    using difference_type = std::ranges::range_difference_t<Base>;

                    constexpr iterator()  = default;
                    constexpr iterator(std::ranges::iterator_t<Base>&& current, Converter&& conv) : m_current(std::move(current)), m_conv(std::move(conv))
                    {}
                    [[nodiscard]] constexpr decltype(auto) operator*() const
                    {
                        return m_conv.template from<std::ranges::iterator_t<Base>>(m_current);
                    }
                    constexpr Self &operator++()
                    {
                        advance();
                        return *this;
                    }
                    constexpr Self operator++(int)
                    {
                        auto tmp = *this;
                        advance();
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
                private:
                    inline void advance()
                    {
                        std::advance(m_current, Converter::GetCharBytes(*m_current));
                    }
                };
                from_view() requires std::default_initializable<Range> = default;
                from_view(const from_view&) = default;
                from_view(from_view&&) = default;
                constexpr from_view(Range rng, Converter conv) : m_rng(std::move(rng)), m_conv(std::move(conv))
                {}

                [[nodiscard]] inline constexpr Range base() const& noexcept {
                    return m_rng;
                }
                [[nodiscard]] inline constexpr Range base() && noexcept {
                    return std::move(m_rng);
                }
                constexpr auto begin() const
                {
                    return iterator{std::ranges::begin(m_rng), Converter{m_conv}};
                }
                constexpr auto begin()
                {
                    return iterator{std::ranges::begin(m_rng), Converter{m_conv}};
                }
                constexpr auto end() const
                {
                    return iterator{std::ranges::end(m_rng), Converter{m_conv}};
                }
                constexpr auto end()
                {
                    return iterator{std::ranges::end(m_rng), Converter{m_conv}};
                }
            };
            template <std::ranges::viewable_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            from_view(Range&&, Converter) -> from_view<std::views::all_t<Range>, Converter>;
            template <typename T>
            struct from_store {
                T c;
            };
            class from_fn
            {
                public:
                    template <std::ranges::viewable_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
                    inline constexpr auto operator()(Range&& rng, Converter&& conv) const noexcept
                    {
                        return from_view(std::forward<Range>(rng), std::forward<Converter>(conv));
                    }
                    template <typename T>
                    inline constexpr auto operator()(T&& conv) const noexcept
                    {
                        return from_store{conv};
                    }
                private:
            };
            template <std::ranges::viewable_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            auto operator|(Range&& rng, from_store<Converter> store)
            {
                return from_view(std::forward<Range>(rng), std::forward<Converter>(store.c));
            }

            template <std::ranges::viewable_range Range, to_converter<std::ranges::iterator_t<Range>> Converter>
            class to_view : public std::ranges::view_interface<to_view<Range, Converter>>{
                Range m_rng;
                Converter m_conv;
            public:
                class iterator {
                    using Self = iterator;
                    using Parent = to_view<Range, Converter>;
                    using Base = Range;
                    std::ranges::iterator_t<Base> m_current;
                    Converter m_conv;
                public:
                    using iterator_concept = rpp::impl::tag_from_range_t<Range>;
                    // using iterator_category = ;
                    using value_type = uint32_t;
                    using difference_type = std::ranges::range_difference_t<Base>;

                    iterator() requires std::default_initializable<std::ranges::iterator_t<Base>> = default;
                    iterator(std::ranges::iterator_t<Base> current, Converter&& c) : m_current(std::move(current)) , m_conv(std::forward(c))
                    {}
                    [[nodiscard]] constexpr decltype(auto) operator*()
                    {
                        return m_conv.template from<std::ranges::iterator_t<Base>>(m_current);
                    }
                    constexpr Self &operator++()
                    {
                        advance();
                        return *this;
                    }
                    constexpr Self &operator++(int)
                    {
                        auto tmp = *this;
                        advance();
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
                private:
                    inline void advance()
                    {
                        m_conv.next();
                        if (m_conv.remains_characters()==0)
                            ++m_current;
                    }
                };
                to_view() requires std::default_initializable<Range>
                {}
                constexpr to_view(Range rng, Converter conv) : m_rng(std::move(rng)), m_conv(conv)
                {}

                [[nodiscard]] inline constexpr Range base() const& noexcept {
                    return m_rng;
                }
                [[nodiscard]] inline constexpr Range base() && noexcept {
                    return std::move(m_rng);
                }
                constexpr auto begin() const
                {
                    return iterator{std::ranges::begin(m_rng), Converter{m_conv}};
                }
                // constexpr auto begin()
                // {
                //     return iterator{std::ranges::begin(m_rng), *this};
                // }
                constexpr auto end() const
                {
                    return iterator{std::ranges::end(m_rng), Converter{m_conv}};
                }
                // constexpr auto end()
                // {
                //     return iterator{std::ranges::end(m_rng), *this};
                // }
            };
            template <std::ranges::viewable_range Range, from_converter<std::ranges::iterator_t<Range>> Converter>
            to_view(Range&&, Converter) -> to_view<std::views::all_t<Range>, Converter>;
            template <typename T>
            struct to_store {
                T c;
            };
            class to_fn
            {
                public:
                    template <std::ranges::viewable_range Range, to_converter<std::ranges::iterator_t<Range>> Converter>
                    inline constexpr auto operator()(Range&& rng, std::remove_reference_t<Converter> conv) const noexcept
                    {
                        return to_view(std::move(rng), std::move(conv));
                    }
                    template <typename T>
                    inline constexpr auto operator()(std::remove_reference_t<T> conv) const noexcept
                    {
                        return to_store{std::move(conv)};
                    }
                private:
            };
            template <std::ranges::viewable_range Range, to_converter<std::ranges::iterator_t<Range>> Converter>
            auto operator|(Range&& rng, to_store<Converter> store)
            {
                return to_view<Range, std::remove_reference_t<Converter>>(std::views::all(rng), std::move(store.c));
            }


        } // namespace impl
        inline constexpr impl::from_fn from;
        inline constexpr impl::to_fn to;
    } // namespace conv
}