#include <vector>
using std::vector;

#define ALIGNUP(nAddress, nBytes) ((((uint)nAddress) + (nBytes) - 1) & (~((nBytes) - 1)))

typedef unsigned char u8;
typedef unsigned int uint;

//用来析构对象
class StackAllocatorDestructor {
private:
    void* dataPtr;
    void(*destructor)(void*);

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
        if (objectNum <= 0) {
            return nullptr;
        }

        size_t objSize = sizeof(ObjectType);
        size_t size = objSize * objectNum;
        ptrdiff_t diff = _apBaseAndCap[1] - _apBaseAndCap[0];

        if (size < diff) {
            //TODO:要考虑内存对齐的哦
            if (allocType == 0) {//从栈底分配
                ObjectType* lowPtr = reinterpret_cast<ObjectType*>(_apBaseAndCap[0]);
                for (size_t index = 0; index < objectNum; index++) {
                    //在栈底空间上构建对象
                    ObjectType* object = ::new (std::addressof(lowPtr[index])) ObjectType(std::forward<Args>(args)...);
                    //将对象注册到析构类中
                    registerObject(object);
                }
                _apBaseAndCap[0] += size;
                return lowPtr;
            } else if (allocType == 1) {//从栈顶分配,指针得往下移动，:)
                ObjectType* highPtr = reinterpret_cast<ObjectType*>(_apBaseAndCap[1]);
                for (size_t index = 1; index <= objectNum; index++) {
                    ObjectType* object = ::new (std::addressof(highPtr - index * objSize)) ObjectType(std::forward<Args>(args)...);
                    registerObject(object);
                }
                _apBaseAndCap[1] -= size;
                return highPtr;
            }
        } else {
            return nullptr;//空间不足
        }
    }
};
