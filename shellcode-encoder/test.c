/*
	Sample encoder/decoder into something similar to base64 (but not the same).

	This is useful if you want to generate ASCII shellcode. All you have to do is
	base the assembly code of the decoder as your primary shellcode decoder.

	NB: You can change base_string to any set of ASCII characters that you like, as long as it's sufficient
	to represent 4 bits of information (i.e. strlen = 32).

	Copyright (c) 2021 by David Shoon

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *base_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


int search_base_string(int c)
{
	for (int i = 0; i < strlen(base_string); i++) {
		if (base_string[i] == c) {
			return i;
		}
	}

	return -1;
}

void decode(char *buf, int len)
{
	int a, b;
	for (int i = 0; i < len; i += 2) {
		a = b = 0;

		a = search_base_string(buf[i]);
		b = search_base_string(buf[i+1]);

		printf("%c", (char) a | ((char) b << 4));
	}
}

void encode(char *buf, int len, char *output)
{
	char str[1024];
	for (int i = 0; i < len; i++) {
		sprintf(str, "%c%c", base_string[buf[i] & 0x0f], base_string[(buf[i] & 0xf0) >> 4]);
		strcat(output, str);
	}
}

int main()
{
	char buf[1024];
	char encoded[2048];

	while (fgets(buf, sizeof(buf), stdin)) {
		memset(encoded, 0, sizeof(encoded));

		encode(buf, strlen(buf), encoded);
		printf("%s\n", encoded);

		decode(encoded, strlen(encoded));
	}
}
