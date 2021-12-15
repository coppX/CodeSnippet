//
// Created by FDC on 2021/12/15.
//

#include "template_if_while.h"
#include <iostream>

int lcm(int a, int b)
{
    int r, lcm = a * b;
    while (r = a % b)
    {
        a = b;
        b = r;
    }
    return lcm/b;
}

int gcd_r(int a, int b)
{
    return b == 0 ? a :gcd_r(b, a % b);
}

int lcm_r(int a, int b)
{
    return a * b / gcd_r(a, b);
}

template<int a, int b>
class lcm_T
{
    template<typename stat>
    class cond
    {
    public:
        enum {ret=(stat::div!=0)};
    };

    template <int a, int b>
    class stat
    {
    public:
        typedef stat<b, a % b> Next;
        enum { div = a % b, ret = b};
    };

    static const int gcd = WHILE_<cond, stat<a, b>>::reType::ret;
public:
    static const int ret = a * b / gcd;
};


template<int a, int b>
class lcm_T_r
{
    template<int a, int b>
    class gcd
    {
    public:
        enum {
            ret = gcd<b, a % b>::ret
        };
    };

    template<int a>
    class gcd<a, 0>
    {
    public:
        enum
        {
            ret = a
        };
    };
public:
    static const int ret = a * b / gcd<a, b>::ret;
};

int main()
{
    std::cout << lcm(100, 36) << '\n';
    std::cout << lcm_r(100, 36) << '\n';
    std::cout << lcm_T<100, 36>::ret << '\n';
    std::cout << lcm_T_r<100, 36>::ret << '\n';
    return 0;
}