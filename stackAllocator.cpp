#include <vector>
#include <unordered_map>
#include <mutex>

using std::vector;
using std::unordered_map;
using std::mutex;

typedef unsigned char u8;
typedef unsigned int uint;

//用来析构对象
class StackAllocatorDestructor {
private:
    void* _dataPtr;
    void(*_destructor)(void*);

public:
    template<typename ObjectPtr>
    StackAllocatorDestructor(ObjectPtr ptr) {
        _dataPtr = static_cast<void*>(ptr);
        _destructor = [](void* objPtr) {
            auto obj = static_cast<ObjectPtr>(objPtr);
            obj->~ObjectPtr();
        };
    }

    void operator()() {
        _destructor(_dataPtr);
    }
};

class StackAllocatorMarker {
private:
    u8* _marker;
    size_t _objNum;
public:
    StackAllocatorMarker(u8* marker, size_t num)
        : _marker(marker), _objNum(num)
    {

    }

    u8* getMarkerPtr() {
        return _marker;
    }

    size_t getMarkerNum() {
        return _objNum;
    }
};

class StackAllocator {
private:
    size_t _allocSize;
    int _nByteAlignment;

    u8* _memeryBase;
    u8* _apBaseAndCap[2];//栈底和栈顶
    u8* _apFrame[2];//低帧和高帧指针

    std::unordered_map<int, vector<StackAllocatorDestructor>> _objectRegister;
    StackAllocatorMarker* _apMarker[2];
    std::mutex _mutex;
public:
    explicit StackAllocator(size_t allocSize, size_t align = 8)
        : _allocSize(allocSize), _nByteAlignment(align)
    {
        _memeryBase = reinterpret_cast<u8*>(allocateAligned(_allocSize, _nByteAlignment));
    }

    ~StackAllocator() {
        if (_memeryBase) {
            freeAligned(_memeryBase);
        }
    }

    void* allocateAligned(size_t size_bytes, size_t alignment) {
        assert(alignment >= 1);
        assert(alignment <= 128);
        assert((alignment & (alignment - 1)) == 0);// alignment是2的幂
        //计算总共要分配的内存量,为了内存对齐偏移量肯定是在0 ~ alignment - 1之间，多分配alignment字节空间肯定没错
        size_t expandSize_bytes = size_bytes + alignment;

        //分配未对齐的内存块，并转换地址为uintptr_t
        uintptr_t rawAddress = reinterpret_cast<uintptr_t>(new u8[expandSize_bytes]);
        
        if (rawAddress) {
            //使用掩码去除地址低位部分，计算错误量，从而计算调整量
            size_t mask = alignment - 1;
            uintptr_t misalignment = (rawAddress & mask);
            ptrdiff_t adjustment = alignment - misalignment;
            
            //计算调整后的地址
            uintptr_t alignedAddress = rawAddress + adjustment;

            //把alignment存储在地址的前一个字节,只取int最低的一个字节就足够
            assert(adjustment < 256);
            u8* pAdjustment = reinterpret_cast<u8*>(alignedAddress);
            pAdjustment[-1] = static_cast<u8>(adjustment);

            _apBaseAndCap[0] = pAdjustment;
            _apBaseAndCap[1] = pAdjustment + expandSize_bytes - adjustment;

            _apFrame[0] = _apBaseAndCap[0];
            _apFrame[1] = _apBaseAndCap[1];

            _apMarker[0] = nullptr;
            _apMarker[1] = nullptr;

            return reinterpret_cast<void*>(alignedAddress);
        } else {
            return nullptr;
        }
    }

    void freeAligned(void *pMen) {
        const u8* pAlignedMem = reinterpret_cast<const u8*>(pMen);
        uintptr_t alignedAddress = reinterpret_cast<uintptr_t>(pMen);

        ptrdiff_t adjustment = static_cast<ptrdiff_t>(pAlignedMem[-1]);

        uintptr_t rawAddress = alignedAddress - adjustment;
        u8* pRawMem = reinterpret_cast<u8*>(rawAddress);
        delete [] pRawMem;
    }

    template<typename ObjectType>
    typename std::enable_if<!std::is_trivially_destructible<ObjectType>::value>::type
        registerObject(int allocType, ObjectType* object) {
        auto iter = _objectRegister.find(allocType);
        if (iter != _objectRegister.end()) {
            iter->second.push_back(StackAllocatorDestructor(object));
        }
    }

    template<typename ObjectType>
    typename std::enable_if<std::is_trivially_destructible<ObjectType>::value>::type
        registerObject(int allocType, ObjectType* object) {

    }

    template<typename ObjectType, typename...Args>
    ObjectType* allocate(int allocType, size_t objectNum, Args...args) {
        if (objectNum <= 0 || !_memeryBase) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(_mutex);

        size_t objSize = sizeof(ObjectType);
        size_t size = objSize * objectNum;
        ptrdiff_t diff = _apFrame[1] - _apFrame[0];

        if (size < diff) {
            if (allocType == 0) {//从栈底分配
                ObjectType* lowPtr = reinterpret_cast<ObjectType*>(_apFrame[0]);
                for (size_t index = 0; index < objectNum; index++) {
                    //在栈底空间上构建对象
                    ObjectType* object = ::new (std::addressof(lowPtr[index])) ObjectType(std::forward<Args>(args)...);
                    //将对象注册到析构类中
                    registerObject(allocType, object);
                }
                _apFrame[0] += size;
                return lowPtr;
            } else {//从栈顶分配,指针得往下移动，:)
                ObjectType* highPtr = reinterpret_cast<ObjectType*>(_apFrame[1]);
                for (size_t index = 1; index <= objectNum; index++) {
                    ObjectType* object = ::new (std::addressof(highPtr[-index])) ObjectType(std::forward<Args>(args)...);
                    registerObject(allocType, object);
                }
                _apFrame[1] -= size;
                return highPtr;
            }
        } else {
            return nullptr;//空间不足
        }
    }

    void setMarker(int allocType) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (allocType == 0) {
            if (_apMarker[0]) delete _apMarker[0];
            _apMarker[0] = new StackAllocatorMarker(_apFrame[0], _objectRegister[0].size());
        } else {
            if (_apMarker[1]) delete _apMarker[1];
            _apMarker[1] = new StackAllocatorMarker(_apFrame[1], _objectRegister[1].size());
        }
    }

    void releaseToMarker(int allocType) {
        std::lock_guard<std::mutex> lock(_mutex);
        u8* ptr;
        size_t objNum;
        if (allocType == 0) {
            ptr = _apMarker[0]->getMarkerPtr();
            objNum = _apMarker[0]->getMarkerNum();
            for (int i = _objectRegister[0].size(); i > objNum; i--) {
                _objectRegister[0].back()();
                _objectRegister[0].pop_back();
            }
            _apFrame[0] = ptr;
        } else {
            ptr = _apMarker[1]->getMarkerPtr();
            objNum = _apMarker[1]->getMarkerNum();
            for (int i = _objectRegister[1].size(); i > objNum; i--) {
                _objectRegister[1].back()();
                _objectRegister[1].pop_back();
            }
            _apFrame[1] = ptr;
        }
    }

    void releaseAll(int allocType) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (allocType == 0) {
            for (int i = _objectRegister[0].size(); i > 0; i--) {
                _objectRegister[0].back()();
                _objectRegister[0].pop_back();
            }
            _apFrame[0] = _apBaseAndCap[0];
        } else {
            for (int i = _objectRegister[1].size(); i > 0; i--) {
                _objectRegister[1].back()();
                _objectRegister[1].pop_back();
            }
            _apFrame[1] = _apBaseAndCap[1];
        }
    }
};
