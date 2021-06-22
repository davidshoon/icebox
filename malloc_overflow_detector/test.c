#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

void do_something(char *p)
{
	printf("%p\n", p); // print something out, to prevent optimizing compiler from skipping this step...
	*p += rand();
	printf("%d\n", *p);
}

void test_crash()
{
	char *p = malloc(1024*1024);

	memset(p, 0, 1024*1024);

	int i;

	for (i = 0; i < 1024*1024; i++) {
		p[i] = rand();
	}

	write(1, "Should crash program\n", strlen("Should crash program\n"));
	i++;
	do_something(p + i);

/*
// We don't really need this anymore, but if it still optimizes our crash out, then we should use this code below...

	write(1, "Ok, this should definitely crash the program\n", strlen("Ok, this should definitely crash the program\n"));

	for (i = 0; ; i++) {
		printf("%d\n", i);
		p[i] = rand();
	}
*/
}

void test_alloc()
{
	char *p = malloc(1024*1024);

	memset(p, 0, 1024*1024);

	free(p);
}

int main()
{
	test_alloc();
	test_crash();
}
