/*

Using C macros to save data-types in a "list" struct.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LIST(xtype, xname) \
struct List_ ## xtype ## _ ## xname \
{ \
	struct List *next, *prev; \
	xtype *data; \
} xname


int main()
{

	LIST(int, my_list);
	LIST(int, my_2nd_list);

}
