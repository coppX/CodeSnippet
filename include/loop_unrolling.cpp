//
// Created by FDC on 2021/12/15.
//
#include <utility>

void bubbleSort(int* data, int n)
{
    for(int i = n - 1; i > 0; --i)
    {
        for(int j = 0; j < i; ++j)
            if(data[j] > data[j + 1])
                std::swap(data[j], data[j + 1]);
    }
}

inline void bubbleSort(int* data)
{
#define COMP_SWAP(i, j) if(data[i] > data[j]) std::swap(data[i], data[j])
    COMP_SWAP(0, 1); COMP_SWAP(1, 2); COMP_SWAP(2, 3);
    COMP_SWAP(0, 1); COMP_SWAP(1, 2);
    COMP_SWAP(0, 1);
}

class recursion {};
void bubbleSort(int* data, int n, recursion)
{
    if(n <= 1) return;
    for (int j = 0; j < n - 1; ++j) if (data[j] > data[j + 1]) std::swap(data[j], data[j + 1]);
    bubbleSort(data, n - 1, recursion());
}

template<int i, int j>
inline void IntSwap(int* data)
{
    if (data[i] > data[j])
        std::swap(data[i], data[j]);
}

template<int i, int j>
inline void IntBubbleSortLoop(int* data)
{
    IntSwap<j, j + 1>(data);
    IntBubbleSortLoop<j < i - 1 ? i : 0, j < i - 1 ? (j + 1 ) : 0 >(data);
}

template<>
inline void IntBubbleSortLoop<0, 0>(int*) { }

template<int n>
inline void IntBubbleSort(int* data)
{
    IntBubbleSortLoop<n - 1, 0>(data);
    IntBubbleSort<n - 1>(data);
}

template<>
inline void IntBubbleSort<1>(int* data);