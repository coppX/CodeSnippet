//
// Created by 最上川 on 2021/12/16.
//

#include "memDetect.h"
int main()
{
    class A {
        int a;
        int b;
    };
    //test
    A* a = new A[10];

    delete [] a;

    int* b = new int;
    delete b;

    return 0;
}