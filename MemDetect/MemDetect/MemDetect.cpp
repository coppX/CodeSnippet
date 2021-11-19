#include <mutex>

#define FILENAME_SIZE 200

#ifndef DEBUG_ALIGNMENT
#define DEBUG_ALIGNMENT 16
#endif

#define ALIGN(s) (((s) + DEBUG_ALIGNMENT - 1) & ~(DEBUG_ALIGNMENT - 1))

struct ptr_list_item {
	ptr_list_item()
		: prev(nullptr)
		, next(nullptr)
		, size(0)
		, line(0)
		, is_array(false)
	{

	}

	ptr_list_item* prev;
	ptr_list_item* next;
	size_t size;
	union 
	{
		char file[FILENAME_SIZE];
		void* addr;
	};
	size_t line;
	bool is_array;
};

#define LIST_ITEM_SIZE  ALIGN(sizeof ptr_list_item)

class MemDetect
{
public:
	//MemDetect();
	//~MemDetect();
	void* operator new(size_t size);
	void operator delete(void *pdead, size_t size);

	void* operator new[](size_t size);
	void operator delete[](void* pdead, size_t size);
private:
	void* allocate(size_t size, char* filename, int line, bool isArray);
	void* deallocate();
	ptr_list_item* list;
	std::mutex m;
};

void* MemDetect::allocate(size_t size, char* filename, int line, bool isArray)
{
	int s = size + LIST_ITEM_SIZE;
	ptr_list_item* raw_ptr = (ptr_list_item*)malloc(s);
	if (nullptr == raw_ptr)
	{
		return nullptr;
	}
	std::lock_guard<std::mutex> lock(m);
	if (nullptr == list)
	{
		list = raw_ptr;
	}
	raw_ptr->size = size;
	raw_ptr->line = line;
	raw_ptr->is_array = isArray;

}

int main()
{
	MemDetect d;
	return 0;
}