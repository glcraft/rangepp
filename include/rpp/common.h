#pragma once
#include <ranges>
namespace rpp
{
    namespace impl
    {
        template <typename range>
        struct tag_from_range {};
        template <std::ranges::random_access_range range>
        struct tag_from_range<range> {
            using type = std::random_access_iterator_tag;
        };
        template <std::ranges::bidirectional_range range>
        struct tag_from_range<range> {
            using type = std::bidirectional_iterator_tag;
        };
        template <std::ranges::forward_range range>
        struct tag_from_range<range> {
            using type = std::forward_iterator_tag;
        };
        template <std::ranges::input_range range>
        struct tag_from_range<range> {
            using type = std::input_iterator_tag;
        };
        template <std::ranges::contiguous_range range>
        struct tag_from_range<range> {
            using type = std::contiguous_iterator_tag;
        };

        template <typename ...Args>
        using tag_from_range_t = typename tag_from_range<Args...>::type;
    } // namespace impl
    
} // namespace rpp

