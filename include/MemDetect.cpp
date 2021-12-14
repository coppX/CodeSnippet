#include <mutex>
#include <cassert>
#include <new>

#define FILENAME_SIZE     200
#define DEBUG_ALIGNMENT   16

#define ALIGN(s) (((s) + DEBUG_ALIGNMENT - 1) & ~(DEBUG_ALIGNMENT - 1))

typedef unsigned char u8;
//static char filename[FILENAME_SIZE] = "\0";
//static int line = 0;

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
	size_t line;
};

#define LIST_ITEM_SIZE  ALIGN(sizeof(ptr_list_item))

class MemDetect
{
public:
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
	memset(raw_ptr, '\0', s);
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
    strcpy(raw_ptr->file, filename);
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
  if (!temp)
  {
    //printf("mac xcode平台上会有一些非应用程序里面的delete操作, 分配内存的时候并不是走的operator new(int, const char*, int),所以在释放的时候并不能从记录的内存信息里面找到，直接释放就好了.\n");
    free(ptr);
    return;
  }
  
	{
		std::lock_guard<std::mutex> lock(m);
		if (temp->prev) temp->prev->next = temp->next;
		if (temp->next) temp->next->prev = temp->prev;
	}
  printf("free memory %d, this size is %d byte\n", (int*)temp, temp->size);
	free(temp);
}

#ifdef _DEBUG
void* operator new(size_t size, const char* filename, int line)
{
  return MemDetect::allocate(size, filename, line);
}

void* operator new[](size_t size, const char* filename, int line)
{
  return MemDetect::allocate(size, filename, line);
}

void operator delete(void* ptr) noexcept
{
  MemDetect::deallocate(ptr);
}

void operator delete[](void* ptr) noexcept
{
  MemDetect::deallocate(ptr);
}

#define new new(__FILE__, __LINE__)
//#define delete strcpy(filename, __FILE__), line = __LINE__, delete
#endif

int main()
{
	int x = _MSC_VER;

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
