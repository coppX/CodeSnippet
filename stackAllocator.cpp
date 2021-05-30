#include <vector>
using std::vector;

#define ALIGNUP(nAddress, nBytes) ((((uint)nAddress) + (nBytes) - 1) & (~((nBytes) - 1)))

typedef unsigned char u8;
typedef unsigned int uint;

//用来析构对象
class StackAllocatorDestructor {
private:
    void* dataPtr;
    void (*destructor)(void*);

public:
    template<typename ObjectPtr>
    StackAllocatorDestructor (ObjectPtr ptr) {
        dataPtr = static_cast<void*>(ptr);
        destructor = [](void* objPtr) {
            auto obj = static_cast<ObjectPtr>(objPtr);
            obj->~ObjectPtr();
        };
    }

    void operator()(){
        destructor(dataPtr);
    }
};

class StackAllocator {
private:
    size_t allocSize_;
    int _nByteAlignment;

    u8 *_memeryBase;
    u8 *_apBaseAndCap[2];
    u8 *_apFrame[2];

    vector<StackAllocatorDestructor> objectRegister;

public:
   explicit StackAllocator(size_t allocSize, size_t align = 8) 
        : allocSize_(ALIGNUP(allocSize, align)), _nByteAlignment(align)
    {
        _memeryBase = new u8[allocSize_];
        if (!_memeryBase) {
            _apBaseAndCap[0] = _memeryBase;
            _apBaseAndCap[1] = _apBaseAndCap[0] + allocSize_;

            _apFrame[0] = _apBaseAndCap[0];
            _apFrame[1] = _apBaseAndCap[1];
        }
    }

    ~StackAllocator() {
        delete [] _memeryBase;
    }

    template<typename ObjectType>
    typename std::enable_if<std::is_trivially_destructible<ObjectType>::value>::type
    registerObject(ObjectType* object) {
        objectRegister.push_back(StackAllocatorDestructor(object));
    }

    template<typename ObjectType>
    typename std::enable_if<!std::is_trivially_destructible<ObjectType>::value>::type
    registerObject(ObjectType* object) {
        
    }

    template<typename ObjectType, typename...Args>
    ObjectType* allocate(int allocType, size_t objectNum, Args...args) {

    }

};
