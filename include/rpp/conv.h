#pragma once
#include "common.h"
#include <ranges>
#include <concepts>
#include "conv/unicode.h"
#include "pipebase.h"
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
            struct from_fn : rpp::impl::pipe_base<from_fn<Container>>
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
            struct to_fn : rpp::impl::pipe_base<to_fn<Container>>
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