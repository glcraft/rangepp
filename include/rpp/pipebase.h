#pragma once
namespace rpp
{
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
    } // namespace impl
    
} // namespace rpp
