#include <mutex>
#include <cassert>

#define FILENAME_SIZE 200

#ifndef DEBUG_ALIGNMENT
#define DEBUG_ALIGNMENT 16
#endif

#define ALIGN(s) (((s) + DEBUG_ALIGNMENT - 1) & ~(DEBUG_ALIGNMENT - 1))

typedef unsigned char u8;

struct ptr_list_item {
	ptr_list_item()
		: prev(nullptr)
		, next(nullptr)
		, size(0)
		, line(0)
	{

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
	//MemDetect();
	//~MemDetect();
	static void* operator new(size_t size);
	static void operator delete(void *pdead, size_t size);

	static void* operator new[](size_t size);
	static void operator delete[](void* pdead, size_t size);
private:
	void* allocate(size_t size, char* filename, int line);
	void deallocate(void* ptr);
	ptr_list_item* list;
	std::mutex m;
};

void* MemDetect::allocate(size_t size, char* filename, int line)
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
			raw_ptr->next = list->next;
			list->next->prev = raw_ptr;
			list = raw_ptr;
		}
	}
	
	raw_ptr->size = size;
	raw_ptr->line = line;
	strncpy(raw_ptr->file, filename, strlen(filename));
	filename[strlen(filename)] = '\0';

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

	free(temp);
}

int main()
{
	MemDetect d;
	return 0;
}