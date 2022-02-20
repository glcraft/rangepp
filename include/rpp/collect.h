#include "pipebase.h"
namespace rpp
{
    namespace impl
    {
        template <typename Container>
        struct collect_fn : impl::pipe_base<collect_fn<Container>>
        {
            template <std::ranges::viewable_range Range>
            inline constexpr auto operator()(Range&& c) const noexcept
            {
                return Container{std::ranges::begin(c), std::ranges::end(c)};
            }
        };
    } // namespace impl
    template <typename Container>
    inline constexpr auto collect = impl::collect_fn<Container>{};
} // namespace rpp
