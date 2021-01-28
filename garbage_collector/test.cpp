#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <set>

std::map <void *, size_t> heap_allocations;
std::set <void *> marked_allocations;

void *my_malloc(size_t size)
{
	void *p = malloc(size);

	printf("Allocated: %p\n", p);

	if (p) {
		heap_allocations[p] = size;
	}

	return p;
}

void my_free(void *ptr)
{
	free(ptr);

	printf("Freed: %p\n", ptr);
}

void sweep()
{
	for (auto x : heap_allocations) {
		auto it = marked_allocations.find(x.first);
		if (it == marked_allocations.end()) {
			my_free(x.first);
		}
	}
}

void heap_mark(std::map <void *, size_t>::iterator it)
{
	marked_allocations.insert(it->first);

	int *ptr = (int *) it->first;

	for (long i = 0; i * sizeof(long) < it->second; i++) {
		auto it = heap_allocations.find(reinterpret_cast <void *> (ptr[i]));
		if (it != heap_allocations.end()) {
			heap_mark(it);
		}
	}
}

void stack_mark()
{
	long esp = (long) &esp;
	int *ptr = (int *) esp;

	marked_allocations.clear();

	for (long i = 0; ptr[i] != 0xdeadbeef; i++) {
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

void func()
{
	struct foo {
		struct foo *next;
		char data[1024];
	};

	struct foo *f = (struct foo *) my_malloc(sizeof(struct foo));

	for (int i = 0; i < 10; i++) {
		f->next = (struct foo *) my_malloc(sizeof(struct foo));
		f = f->next;
	}
}

int main()
{
	long stack_begin = 0xc001cafedeadbeef;

	for (int i = 0; i < 100; i++) {
		func(); // leaks memory.
	}

	gc();
}
