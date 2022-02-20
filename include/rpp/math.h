#pragma once
#include "common.h"
#include <ranges>
#include <functional>

namespace rpp
{
    namespace math
    {
        template <typename Op, typename T>
        concept binary_operator = requires (Op op, T a, T b) {
            {op(a,b)} -> std::convertible_to<T>;
        };
        template <typename Op, typename T>
        concept unary_operator = requires (Op op, T a) {
            {op(a)} -> std::convertible_to<T>;
        };
        template <std::ranges::input_range Range, binary_operator<typename Range::value_type> Op>
        class binary_op_view : public std::ranges::view_interface<binary_op_view<Range, Op>>
        {
        public:
            using value_type = typename Range::value_type;
            using operator_type = Op;

            class iterator {
                using Self = iterator;
                using Parent = binary_op_view<Range, Op>;
                using Base = Range;
            public:
                using value_type = typename Range::value_type;
                using operator_type = Op;
                
                // using iterator_concept = typename tag_from_range_t<Range>;
                // using iterator_category = ;
                // using value_type = typename Base::value_type;
                using difference_type = std::ranges::range_difference_t<Base>;

                iterator() requires std::default_initializable<std::ranges::iterator_t<Base>> = default;
                iterator(std::ranges::iterator_t<Base> current) : m_current(std::move(current)) 
                {}
                iterator(std::ranges::iterator_t<Base> current, value_type n) : m_current(std::move(current)), m_adds(n), m_op()
                {}
                
                constexpr decltype(auto) operator*() const
                {
                    return m_op(*m_current, m_adds);
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
                constexpr Self &operator++(int) requires std::ranges::forward_range<Base>
                {
                    auto tmp = *this;
                    ++*this;
                    return tmp;
                }
                // constexpr Self &operator--()
                // {
                //     --m_current;
                //     return this;
                // }
                // constexpr Self &operator--(int) requires std::ranges::forward_range<Base>
                // {
                //     auto tmp = *this;
                //     --*this;
                //     return tmp;
                // }
                constexpr Self &operator+=(difference_type n) requires std::ranges::random_access_range<Base>
                {
                    m_current += n;
                    return *this;
                }
                // constexpr Self &operator-=(difference_type n) requires std::ranges::random_access_range<Base>
                // {
                //     m_current -= n;
                //     return *this;
                // }
                [[nodiscard]] friend constexpr bool operator==(const Self &x, const Self &y)
                {
                    return x.m_current == y.m_current;
                }
            private:
                std::ranges::iterator_t<Base> m_current;
                value_type m_adds;
                operator_type m_op;
            };
            constexpr binary_op_view() requires std::default_initializable<Range>
            {}
            constexpr binary_op_view(Range rng, typename Range::value_type n) : m_rng(std::move(rng)), m_adds(n)
            {}

            [[nodiscard]] inline constexpr Range base() const& noexcept {
                return m_rng;
            }
            [[nodiscard]] inline constexpr Range base() && noexcept {
                return std::move(m_rng);
            }
            constexpr auto begin() const
            {
                return iterator{std::ranges::begin(m_rng), m_adds};
            }
            constexpr auto begin()
            {
                return iterator{std::ranges::begin(m_rng), m_adds};
            }
            constexpr auto end() const
            {
                return iterator{std::ranges::end(m_rng), m_adds};
            }
            constexpr auto end()
            {
                return iterator{std::ranges::end(m_rng), m_adds};
            }
        private:
            Range m_rng;
            value_type m_adds;
        };
        template <typename T, binary_operator<T> Op, T N>
        class binary_op_fn
        {
            public:
                template <std::ranges::input_range Range>
                inline constexpr auto operator()(Range&& rng) const noexcept
                {
                    return binary_op_view(std::move(rng), static_cast<T>(N));
                }
            private:
        };
        template <std::ranges::input_range Range, binary_operator<typename Range::value_type> Op, typename Range::value_type N>
        binary_op_view<Range, Op> operator|(Range rng, binary_op_fn<typename Range::value_type, Op, N>)
        {
            return binary_op_view<Range, Op>(std::move(rng), static_cast<typename Range::value_type>(N));
        }
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::plus<decltype(N)>, N> add_const;
        template <typename T>
        inline constexpr binary_op_fn<T, std::plus<T>, static_cast<T>(1)> increment;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::minus<decltype(N)>, N> minus_const;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::multiplies<decltype(N)>, N> mult_const;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::divides<decltype(N)>, N> div_const;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::modulus<decltype(N)>, N> mod_const;

        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::bit_and<decltype(N)>, N> bit_and_const;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::bit_or<decltype(N)>, N> bit_or_const;
        template <auto N>
        inline constexpr binary_op_fn<decltype(N), std::bit_xor<decltype(N)>, N> bit_xor_const;
    }
}