//
// Created by FDC on 2021/12/14.
//

#include "template_if_while.h"
#include <iostream>

int main()
{
    const int len = 4;
    typedef
        IF_<sizeof(short)==len, short,
        IF_<sizeof(int)==len, int,
        IF_<sizeof(long)==len, long,
        IF_<sizeof(long long)==len, long long,void>::reType>::reType>::reType>::reType int_my;
    std::cout << sizeof(int_my) << '\n';
}