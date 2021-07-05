#include "rpp.h"
#include <array>
#include <vector>
#include <iostream>


// template <class _Ty>
// concept weakly_incrementable = default_initializable<_Ty> && movable<_Ty> && requires(_Ty __i) {
//     typename iter_difference_t<_Ty>;
//     requires _Signed_integer_like<iter_difference_t<_Ty>>;
//     { ++__i } -> same_as<_Ty&>;
//     __i++;
// };



template <std::weakly_incrementable T>
void jhfds(T)
{}
int main()
{
    using namespace rpp::conv;
    auto v = std::string("Héllöworld");
    auto t = static_cast<const std::string&>(v) | from(utf8);// | to(utf8);
    // wrong<decltype(std::_Fake_decay_copy(t.begin()))> b;
    // static_assert(std::ranges::_Begin::_Has_member<decltype(t)>);
    // static_assert(std::weakly_incrementable<decltype(std::_Fake_decay_copy(t.begin()))>);
    jhfds(t.begin());
    auto test = std::ranges::begin(t);
    // auto t = static_cast<const std::string&>(v) | std::views::transform([](char c) { return c; });

    for(auto i : t)
    {
        std::cout << std::dec<<i << '(' << std::hex << i << ") ";
        if (i<0x80)
            std::cout <<(char)i;
        std::cout << std::endl;
    }
    return 0;
}