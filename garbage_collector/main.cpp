#include "gc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func()
{
	struct foo {
		struct foo *next;
		char data[1024];
	};

	struct foo *f = (struct foo *) my_malloc(sizeof(struct foo));
	struct foo *root = f;

	for (int i = 0; i < 10; i++) {
		f->next = (struct foo *) my_malloc(sizeof(struct foo));
		f = f->next;
	}

	f->next = root;
}

int main()
{
	long stack_begin = my_stack_marker();

	for (int i = 0; i < 100; i++) {
		func(); // leaks memory.
	}

	gc();
	gc(); // call it twice to prove that there's no problems with freeing ... should do nothing on this round.
}

