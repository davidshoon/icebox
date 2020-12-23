/*

	This program generates an XOR checksum, which is useful for error correction purposes.

	The amount of contiguous error correction that it's capable of is determined by the "size" argument.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

struct checksum_list
{
	struct checksum_list *next;
	uint32_t checksum;
};

struct checksum_list g_checksum_list; // dummy root node linked list.

struct checksum_list *add_checksum(struct checksum_list *curr, uint32_t checksum)
{
	curr->next = malloc(sizeof(struct checksum_list));

	if (curr->next == NULL) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}

	curr->next->next = NULL;
	curr->next->checksum = checksum;

	return curr->next;
}


int main(int argc, char **argv)
{
	FILE *fp;
	uint32_t total;
	char *buf, *xor;
	size_t i, r;
	uint32_t checksum;
	struct checksum_list *p_checksum_list = &g_checksum_list;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <file> <size of xor>\n", argv[0]);
		fprintf(stderr, "Where <file> can be an ordinary file, a device file of a disk, or a partition of a disk.\n");
		exit(1);
	}

	fp = fopen(argv[1], "rb");

	if (!fp) {
		perror("fopen");
	}

	total = atoi(argv[2]);

	buf = malloc(total);
	xor = malloc(total);

	if (!buf || !xor) {
		fprintf(stderr, "Can't allocate memory\n");
		exit(2);
	}

	memset(buf, 0, total);
	memset(xor, 0, total);

	fwrite(&total, sizeof(total), 1, stdout); // write out size of xor_sum -- this is the header.

	// next, we write out a checksum for each block of data (which is the size of xor).
	// NB: In the future you can replace this with a cryptographic hash, if you want.

	while ((r = fread(buf, 1, total, fp))) {
		checksum = 0;
		for (i = 0; i < r; i++) {
			xor[i] ^= buf[i]; // our rolling xor 
			checksum += buf[i]; // our checksum.
		}

		p_checksum_list = add_checksum(p_checksum_list, checksum);

		if (r < total) {
			fprintf(stderr, "Finished\n");
			break;
		}
	}

	// calculate total number of checksums in our linked list, excluding dummy root node.

	for (checksum = 0, p_checksum_list = g_checksum_list.next; p_checksum_list != NULL; p_checksum_list = p_checksum_list->next, checksum++)
		;

	fwrite(&checksum, sizeof(checksum), 1, stdout); // write out number of checksums...

	for (p_checksum_list = g_checksum_list.next; p_checksum_list != NULL; p_checksum_list = p_checksum_list->next) {
		checksum = p_checksum_list->checksum;
		fwrite(&checksum, sizeof(checksum), 1, stdout); // write out checksum for each block
	}


	// write out our XOR block
	for (i = 0; i < total; i++) {
		printf("%c", xor[i]);
	}

	fclose(fp);

	return 0;
}
