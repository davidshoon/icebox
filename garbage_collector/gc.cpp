#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <set>
#include <list>

static void init() __attribute__((constructor));
static void fini() __attribute__((destructor));

static void init()
{
	printf("Loaded garbage collector\n");
}


static void fini()
{
	printf("Unloaded garbage collector\n");
}


long my_stack_marker()
{
	return 0xc001cafedeadbeef;
}

static std::map <void *, size_t> heap_allocations;
static std::set <void *> marked_allocations;

void *my_malloc(size_t size)
{
	void *p = malloc(size);

	printf("Allocated: %p\n", p);

	if (p) {
		heap_allocations[p] = size;
	}

	return p;
}

static void my_free(void *ptr)
{
	free(ptr);

	printf("Freed: %p\n", ptr);
}

static void sweep()
{
	auto heap_next = heap_allocations.begin();

	for (auto heap_it = heap_allocations.begin(); heap_it != heap_allocations.end(); heap_it = heap_next) {
		auto x = *heap_it;
		auto it = marked_allocations.find(x.first);
		if (it == marked_allocations.end()) {
			heap_next++;
			heap_allocations.erase(heap_it);
			my_free(x.first);
		}
		else {
			heap_next++;
		}
	}
}

static void heap_mark(std::map <void *, size_t>::iterator it)
{
	marked_allocations.insert(it->first);

	long *ptr = (long *) it->first;

	for (long i = 0; i * sizeof(long) < it->second; i++) {
		auto it = heap_allocations.find(reinterpret_cast <void *> (ptr[i]));
		if (it != heap_allocations.end()) {
			auto jt = marked_allocations.find(reinterpret_cast <void *> (ptr[i]));
			if (jt == marked_allocations.end()) {
				heap_mark(it);
			}
		}
	}
}

static void stack_mark()
{
	long esp = (long) &esp;
	long *ptr = (long *) esp;

	marked_allocations.clear();

	for (long i = 0; ptr[i] != my_stack_marker(); i++) {
		auto it = heap_allocations.find(reinterpret_cast <void *> (ptr[i]));
		if (it != heap_allocations.end()) {
			heap_mark(it);
		}
	}
}

void gc()
{
	stack_mark();
	sweep();
}

