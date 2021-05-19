#include <cassert>
template<typename T>
class Singleton {
    static T* ms_Singleton;
public:
    /*这里计算出派生类实例的相对位置，因为这里派生类可能不仅仅是从singleton派生，这种情况下，MyClass的this
    可能与singleton的this不同。这种解决方法假设一个不存在的对象在内存0x1位置上，将此对象强制转换为两种类型，
    并得到其偏移量的差值。这个差值可以有效地作为Singleton<MyClass>和它的拍摄类型MyClass的距离，可用于计算
    singleton的指针。
    */
    Singleton() {
        assert(!ms_Singleton);
        int offset = (int)(T*)1 - (int)(Singleton<T>*)(T*)1;
        ms_Singleton = (T*)((int)this + offset);
    }
    ~Singleton() {
        assert(ms_Singleton);
        ms_Singleton = 0;
    }
    static T& GetSingleton() {
        assert(ms_Singleton);
        return (*ms_Singleton);
    }
    static T* GetSingletonPtr() {
        return (ms_Singleton);
    }
};