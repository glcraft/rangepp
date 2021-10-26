#pragma once
#include "common.h"
#include <ranges>
#include <concepts>
#include "conv/utf8.h"

namespace rpp
{
    namespace conv
    {
        template <typename Conv>
        concept converter = std::forward_iterator<Conv> 
            && requires {
                typename Conv::input_type;
                typename Conv::output_type;
            };
        template <typename Conv>
        concept from_converter = converter<Conv> && std::same_as<typename Conv::output_type, uint32_t>;
        template <typename Conv>
        concept to_converter = converter<Conv> &&  std::same_as<typename Conv::input_type, uint32_t>;
        template <typename Container, typename Range>
        concept from_container = std::ranges::viewable_range<Range> && from_converter<typename Container::template from<Range>>;
        template <typename Container, typename Range>
        concept to_container = std::ranges::viewable_range<Range> && to_converter<typename Container::template to<Range>>;
        

        namespace impl
        {
            template <class Left, class Right>
            concept CanPipe = requires(Left&& a, Right&& b) {
                static_cast<Right&&>(b)(static_cast<Left&&>(a));
            };
            template <class Left, class Right>
            concept CanPipeRef = requires(Left& a, Right&& b) {
                static_cast<Right&&>(b)(std::views::all(a));
            };
            template <class Left, class Right>
            concept CanCompose = std::constructible_from<std::remove_cvref_t<Left>, Left> && std::constructible_from<std::remove_cvref_t<Right>, Right>;
            
            template <class, class>
            class Pipeline;

            template <class Derived>
            struct pipe_base
            {
                template <CanCompose<Derived> Right>
                constexpr auto operator|(const pipe_base<Right>& right) &&
                {
                    return Pipeline{static_cast<Derived&&>(*this), static_cast<Right&&>(right)};
                }
                template <CanCompose<Derived> Right>
                constexpr auto operator|(pipe_base<Right>&& right) &&
                {
                    return Pipeline{static_cast<Derived&&>(*this), static_cast<Right&&>(right)};
                }
                template <CanPipe<const Derived&> Left>
                friend constexpr auto operator|(Left&& left, const pipe_base& right)
                {
                    return static_cast<const Derived&>(right)(std::forward<Left>(left));
                }
                template <CanPipe<Derived> Left>
                friend constexpr auto operator|(Left&& left, pipe_base&& right)
                {
                    return static_cast<Derived&&>(right)(std::forward<Left>(left));
                }

                template <CanPipeRef<Derived> Left>
                friend constexpr auto operator|(const Left& left, pipe_base&& right)
                {
                    return static_cast<Derived&&>(right)(std::views::all(left));
                }
                template <CanPipeRef<const Derived&> Left>
                friend constexpr auto operator|(const Left& left, const pipe_base& right)
                {
                    return static_cast<const Derived&>(right)(std::views::all(left));
                }
            };

            template <class Left, class Right>
            class Pipeline : pipe_base<Pipeline<Left, Right>> 
            {
                Left l;
                Right r;
            public:
                constexpr Pipeline(Left&& l, Right&& r) : l(std::forward<Left>(l)), r(std::forward<Left>(r))
                {}
                template <typename T>
                constexpr auto operator|(T&& val) 
                {
                    return r(l(std::forward(val)));
                }
                template <typename T>
                constexpr auto operator|(T&& val) const
                {
                    return r(l(std::forward(val)));
                }
            };
            template <class Left, class Right>
            Pipeline(Left, Right) -> Pipeline<Left, Right>;

            template <std::ranges::viewable_range Range, from_converter Converter>
            class from_view : public std::ranges::view_interface<from_view<Range, Converter>>{
                Range m_rng;
            public:
                using iterator = Converter;
                constexpr from_view() requires std::default_initializable<Range> = default;
                
                constexpr from_view(Range&& rng) : m_rng(std::forward<Range>(rng))
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
            template <std::ranges::viewable_range Range, from_converter Converter>
            from_view(Range&&, Converter) -> from_view<std::views::all_t<Range>, Converter>;

            template <typename Container>
            struct from_fn : pipe_base<from_fn<Container>>
            {
                template <std::ranges::viewable_range Range>
                    requires from_container<Container, Range>
                inline constexpr auto operator()(Range&& rng) const noexcept
                {
                    return from_view<Range, typename Container::template from<Range>>(std::forward<Range>(rng));
                }
            };
            template <std::ranges::viewable_range Range, to_converter Converter>
            class to_view : public std::ranges::view_interface<to_view<Range, Converter>>{
                Range m_rng;
            public:
                using iterator = Converter;
                constexpr to_view() requires std::default_initializable<Range> = default;
                
                constexpr to_view(Range&& rng) : m_rng(std::forward<Range>(rng))
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
                    return iterator(std::ranges::begin(m_rng));
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

            
            
            template <typename Container>
            struct to_fn : pipe_base<to_fn<Container>>
            {
                template <std::ranges::viewable_range Range>
                    requires to_container<Container, Range>
                inline constexpr auto operator()(Range&& rng) const noexcept
                {
                    return to_view<Range, typename Container::template to<Range>>(std::forward<Range>(rng));
                }
            };
        }
        template <typename Converter>
        inline constexpr impl::from_fn<Converter> from;
        template <typename Converter>
        inline constexpr impl::to_fn<Converter> to;
    } // namespace conv
}