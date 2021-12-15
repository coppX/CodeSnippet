//
// Created by FDC on 2021/12/14.
//
#include "template_if_while.h"
#include <iostream>

// 计算 1^e+2^e+...+n^e
template<int n, int e>
class sum_pow{

    template<int i, int e1>
    class pow_e{
    public:
        enum{ ret = i * pow_e<i, e1 - 1>::ret };
    };

    template<int i>
    class pow_e<i, 0>{
    public:
        enum{ ret = 1 };
    };

    // 计算 i^e，嵌套类使得能够定义嵌套模板元函数，private 访问控制隐藏实现细节
    template<int i>
    class pow{
    public:
        enum{ ret = pow_e<i, e>::ret };
    };

    template<typename stat>
    class cond {
    public:
        enum{ ret = (stat::ri <= n) };
    };

    template<int i, int sum>
    class stat {
    public:
        typedef stat<i + 1, sum + pow<i>::ret> Next;
        enum{ ri = i, ret = sum };
    };
public:
    enum{ ret = WHILE_<cond, stat<1, 0>>::reType::ret };
};

int main()
{
    std::cout << sum_pow<10, 2>::ret << '\n';
    return 0;
}