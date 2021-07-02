#pragma once
#include <ranges>
namespace rpp
{
    namespace impl
    {
        template <typename range, typename ...Args>
        struct tag_from_range {
            using type = tag_from_range<Args...>;
        };
        template <std::ranges::random_access_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::random_access_iterator_tag;
        };
        template <std::ranges::bidirectional_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::bidirectional_iterator_tag;
        };
        template <std::ranges::forward_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::forward_iterator_tag;
        };
        template <std::ranges::input_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::input_iterator_tag;
        };
        template <std::ranges::contiguous_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::contiguous_iterator_tag;
        };
        template <typename range>
        struct tag_from_range<range> {};

        template <typename ...Args>
        using tag_from_range_t = tag_from_range<Args...>;
    } // namespace impl
    
} // namespace rpp

