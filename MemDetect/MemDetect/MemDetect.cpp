#include "stackAllocator.h"

using std::size_t;

struct new_ptr_list_t {
	new_ptr_list_t* prev;
	new_ptr_list_t* next;
	size_t size;
	union 
	{
		char file[200];
		void* addr;
	};
	size_t line;
	bool is_array;
};

class MemDetect
{
public:
	MemDetect();
	~MemDetect();
	void* operator new(size_t size);
	void operator delete(void *pdead, size_t size);

	void* operator new[](size_t size);
	void operator delete[](void* pdead, size_t size);
private:
	new_ptr_list_t* ptr_list;
};