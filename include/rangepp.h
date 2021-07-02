#include <ranges>

namespace rpp
{
    using namespace std::ranges;
    namespace impl
    {
        template <typename range, typename ...Args>
        struct tag_from_range {
            using type = tag_from_range<Args...>;
        };
        template <random_access_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::random_access_iterator_tag;
        };
        template <bidirectional_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::bidirectional_iterator_tag;
        };
        template <forward_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::forward_iterator_tag;
        };
        template <input_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::input_iterator_tag;
        };
        template <contiguous_range range, typename ...Args>
        struct tag_from_range<range, Args...> {
            using type = std::contiguous_iterator_tag;
        };
        template <typename range>
        struct tag_from_range<range> {};

        template <typename ...Args>
        using tag_from_range_t = tag_from_range<Args...>;
        
        template <input_range Range>
        class plus_one_view : public view_interface<plus_one_view<Range>>{
            Range m_rng;
        public:
            class iterator {
                using Self = iterator;
                using Parent = plus_one_view<Range>;
                using Base = Range;
                iterator_t<Base> m_current;
            public:
                using iterator_concept = tag_from_range_t<Range>;
                // using iterator_category = ;
                using value_type = typename Base::value_type;
                using difference_type = range_difference_t<Base>;

                iterator() requires std::default_initializable<iterator_t<Base>> = default;
                iterator(iterator_t<Base> current) : m_current(std::move(current)) 
                {}
                constexpr decltype(auto) operator*() const
                {
                    return *m_current + 1;
                }
                constexpr Self& operator++()
                { 
                    ++m_current; 
                    return *this; 
                    }
                constexpr void operator++(int)
                { 
                    ++m_current; 
                    }
                constexpr Self& operator++(int) requires forward_range<Base>
                { auto tmp = *this; ++*this; return tmp; }
                constexpr Self& operator--()
                { --m_current; return this; }
                constexpr Self& operator--(int) requires forward_range<Base>
                { auto tmp = *this; --*this; return tmp; }
                constexpr Self& operator+=( difference_type n ) requires random_access_range<Base>
                { m_current+=n; return *this; }
                constexpr Self& operator-=( difference_type n ) requires random_access_range<Base>
                { m_current-=n; return *this; }
                [[nodiscard]] friend constexpr bool operator==( const Self& x, const Self& y )
                {
                    return x.m_current == y.m_current;
                }
            };
            plus_one_view() requires std::default_initializable<Range>
            {}
            constexpr plus_one_view(Range rng) : m_rng(std::move(rng))
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
        
        class plus_one_fn
        {
            public:
                template <typename Range>
                    requires input_range<Range>
                inline constexpr auto operator()(Range&& rng) const noexcept
                {
                    return plus_one_view(std::move(rng));
                }
            private:
        };
        template <input_range Range>
        plus_one_view<Range> operator|(Range rng, plus_one_fn)
        {
            return plus_one_view(std::move(rng));
        }
    } // namespace impl
    inline constexpr impl::plus_one_fn plus_one;
    
    // template <typename Range, typename Predicate>
    //     requires input_range<Range>
    // class view_split_when : public view_base
    // {
    // public:
    //     using base = iterator_t<Range>;
    //     constexpr Base base() const& requires std::copy_constructible<V>;
    //     constexpr Base base() const&&;
    // private:
    //     Base b;
    // };
}