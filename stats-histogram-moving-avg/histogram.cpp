#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int table[256] = {0};

int main()
{
	int c;

	while ((c = getc(stdin)) != EOF) {
		table[c]++;
	}

	for (int i = 0; i < 256; i++) {
		printf("%d: %d\t", i, table[i]);
	}

	printf("\n");
}
