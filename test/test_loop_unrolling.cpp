//
// Created by FDC on 2021/12/15.
//

#include <iostream>
#include <omp.h>
#include "loop_unrolling.h"

int main()
{
    double t1, t2, t3;
    const int num = 100000000;

    int data[4];
    int inidata[4] = {3, 4, 2, 1};
    t1 = omp_get_wtime();
    for (int i = 0; i < num; i++)
    {
        memcpy(data, inidata, 4);
        bubbleSort(data, 4);
    }
    t1 = omp_get_wtime() - t1;
    t2 = omp_get_wtime();
    for (int i = 0; i < num; i++)
    {
        memcpy(data, inidata, 4);
        bubbleSort4(data);
    }
    t2 = omp_get_wtime() - t2;
    t3 = omp_get_wtime();
    for (int i = 0; i < num; i++)
    {
        memcpy(data, inidata, 4);
        IntBubbleSort<4>(data);
    }
    t3 = omp_get_wtime() - t3;

    std::cout << t1 / t3 << '\t' << t2 / t3 << '\n';
    std::cin.get();

    return 0;
}