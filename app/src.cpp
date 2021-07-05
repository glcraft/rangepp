#include "rpp.h"
#include <array>
#include <vector>
#include <iostream>
int main()
{
    using namespace rpp::conv;
    auto v = std::string("Héllöworld");
    auto t = v | from(utf8);

    for(auto i : t)
    {
        std::cout << std::dec<<i << '(' << std::hex << i << ") ";
        if (i<0x80)
            std::cout <<(char)i;
        std::cout << std::endl;
    }
    return 0;
}