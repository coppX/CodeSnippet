#include <mutex>
#include <cassert>
#include <new>

#define FILENAME_SIZE     200
#define DEBUG_ALIGNMENT   16

#define ALIGN(s) (((s) + DEBUG_ALIGNMENT - 1) & ~(DEBUG_ALIGNMENT - 1))

typedef unsigned char u8;

struct ptr_list_item {
	ptr_list_item()
		: prev(nullptr)
		, next(nullptr)
		, size(0)
		, line(0)
	{
        memset(file, '\0', FILENAME_SIZE);
	}

	ptr_list_item* prev;
	ptr_list_item* next;
	size_t size;

	char file[FILENAME_SIZE];
	void* addr;
	size_t line;
};

#define LIST_ITEM_SIZE  ALIGN(sizeof(ptr_list_item))

class MemDetect
{
public:
	static void* operator new(size_t size, const char* file, int line);
	static void operator delete(void *pdead);

	static void* operator new[](size_t size, const char* file, int line);
	static void operator delete[](void* pdead);

private:
	static void* allocate(size_t size, const char* filename, int line);
	static void deallocate(void* ptr);
	static ptr_list_item* list;
	static std::mutex m;
};
ptr_list_item* MemDetect::list = nullptr;
std::mutex MemDetect::m = {};

void* MemDetect::allocate(size_t size, const char* filename, int line)
{
	int s = size + LIST_ITEM_SIZE;
	ptr_list_item* raw_ptr = reinterpret_cast<ptr_list_item*>(malloc(s));
	assert(raw_ptr != nullptr);

	u8* user_ptr = reinterpret_cast<u8*>(raw_ptr) + LIST_ITEM_SIZE;

	{
		std::lock_guard<std::mutex> lock(m);
		if (nullptr == list)
		{
			list = raw_ptr;
		}
		else
		{
            raw_ptr->next = list;
            list->prev = raw_ptr;
			list = raw_ptr;
		}
	}
	
	raw_ptr->size = size;
	raw_ptr->line = line;
    if (nullptr != filename)
    {
        strncpy(raw_ptr->file, filename, strlen(filename));
        raw_ptr->file[strlen(filename)] = '\0';
    }

	return reinterpret_cast<void*>(user_ptr);
}

void MemDetect::deallocate (void* ptr)
{
	u8* raw_ptr = reinterpret_cast<u8*>(ptr) - LIST_ITEM_SIZE;
	
	ptr_list_item* list_ptr = reinterpret_cast<ptr_list_item*>(raw_ptr);
	ptr_list_item* temp = list;
	while (temp && temp != list_ptr)
	{
		temp = temp->next;
	}

	if (nullptr == temp)
	{
		printf("can't find the ptr in the list!");
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m);
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
	}
    printf("free memory %d, this size is %d byte\n", (int*)temp, temp->size);
	free(temp);
}

void* MemDetect::operator new(size_t size, const char* filename, int line)
{
  return allocate(size, filename, line);
}

void* MemDetect::operator new[](size_t size, const char* filename, int line)
{
  return allocate(size, filename, line);
}

void MemDetect::operator delete(void* pdead)
{
  deallocate(pdead);
}

void MemDetect::operator delete[](void* pdead)
{
  deallocate(pdead);
}

#ifdef DEBUG
void* operator new(size_t size)
{
  return MemDetect::operator new(size, nullptr, 0);
}

void* operator new[](size_t size)
{
  return MemDetect::operator new[](size, nullptr, 0);
}

void operator delete(void* ptr) noexcept
{
  MemDetect::operator delete(ptr);
}

void operator delete[](void* ptr) noexcept
{
  MemDetect::operator delete[](ptr);
}

void* operator new(size_t size, std::nothrow_t&) noexcept
{
  return MemDetect::operator new(size, nullptr, 0);
}

void* operator new[](size_t size, std::nothrow_t&) noexcept
{
  return MemDetect::operator new[](size, nullptr, 0);
}

void operator delete(void* ptr, std::nothrow_t&) noexcept
{
  MemDetect::operator delete(ptr);
}

void operator delete[](void* ptr, std::nothrow_t&) noexcept
{
  MemDetect::operator delete[](ptr);
}
#endif

int main()
{
    //test
    int* a = new int[10];
	MemDetect d;
	return 0;
}
