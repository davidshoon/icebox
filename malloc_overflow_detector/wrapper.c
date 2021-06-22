#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stddef.h>
#include <stdint.h>

#include <sys/mman.h>
#include <unistd.h>

struct chunk_header
{
	void *start_of_chunk;
	void *fence;
	size_t size;
} __attribute__((packed));

static void *my_malloc(size_t size)
{
	struct chunk_header *header;
	static long pagesize = 0;

	if (pagesize == 0) {
		pagesize = sysconf(_SC_PAGE_SIZE);
	}

	if (pagesize == -1) {
		perror("sysconf");
		exit(1);
	}

	write(1, "my_malloc begin\n", strlen("my_malloc begin\n"));

	size_t alloc_size = size + sizeof(struct chunk_header);

	size_t pages = alloc_size / pagesize;
	size_t pagesremainder = alloc_size % pagesize;

	if (pagesremainder != 0) {
		pages++;
	}

	pages++; // electric fence -- add another page for protection boundary...

	void *p = mmap(NULL, pages * pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (p == MAP_FAILED) {
		abort();
	}


	header = (struct chunk_header *) ((char *) p + pagesize - pagesremainder);
	header->start_of_chunk = p;
	header->fence = (char *) p + pagesize * (pages - 1);
	header->size = size;

	p = (char *) p + sizeof(struct chunk_header) + (pagesize - pagesremainder);

	if (mprotect(header->fence, pagesize, PROT_READ) == -1) {
		perror("mprotect");
		exit(1);
	}

	write(1, "my_malloc end\n", strlen("my_malloc end\n"));

	return p;
}

static void my_free(void *ptr)
{
	struct chunk_header *header = (struct chunk_header *) ((char *) ptr - sizeof(struct chunk_header));

	write(1, "my_free begin\n", strlen("my_free begin\n"));

	if (munmap(header->start_of_chunk, header->size) < 0) {
		abort();
	}

	write(1, "my_free end\n", strlen("my_free end\n"));

	return;
}

void *malloc(size_t size)
{
	return my_malloc(size);
}

void *calloc(size_t num, size_t size)
{
	return my_malloc(num * size);
}

void *realloc(void *p, size_t size)
{
	struct chunk_header *header = (struct chunk_header *) ((char *) p - sizeof(struct chunk_header));

	write(1, "realloc begin\n", strlen("realloc begin\n"));

	if (size > header->size) {
		void *new_ptr = my_malloc(size);
		memcpy(new_ptr, p, header->size);
		my_free(p);
		write(1, "realloc end new_ptr", strlen("realloc end new_ptr\n"));
		return new_ptr;
	}

	write(1, "realloc end\n", strlen("realloc end\n"));

	return p;
}

void free(void *p)
{
	my_free(p);
}

