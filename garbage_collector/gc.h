#pragma once

#include <stdlib.h>

extern long my_stack_marker();
extern void *my_malloc(size_t size);
extern void gc();
