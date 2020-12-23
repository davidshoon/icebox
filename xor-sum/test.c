/*

	This program uses an XOR checksum, to do error correction.

	Warning: it will alter the target file with the fixes!

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdint.h>

void load_xor_file(const char *filename, uint32_t *xorsize, char **xor, uint32_t *num_checksums, uint32_t **checksums)
{
	FILE *fp;
	size_t r;
	uint32_t i;
	uint32_t checksum;

	fp = fopen(filename, "rb");
	if (!fp) {
		perror("fopen");
		exit(2);
	}

	// Read xor size from file.

	r = fread(xorsize, sizeof(*xorsize), 1, fp);
	if (r == 0) {
		perror("fread");
		exit(3);
	}

	// Read number of checksums from file.

	r = fread(num_checksums, sizeof(*num_checksums), 1, fp);
	if (r == 0) {
		perror("fread");
		exit(4);
	}

	*checksums = calloc(*num_checksums, sizeof(uint32_t));

	if (*checksums == NULL) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(5);
	}

	// Read checksums from file

	for (i = 0; i < *num_checksums; i++) {
		if (fread(&checksum, sizeof(checksum), 1, fp) == 0) {
			perror("fread");
			exit(5);
		}

		(*checksums)[i] = checksum;
	}

	*xor = malloc(*xorsize);

	if (*xor == NULL) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(6);
	}

	// Read xor block from file.

	if (fread(xor, *xorsize, 1, fp) == 0) {
		perror("fread");
		exit(6);
	}

	return;
}


int main(int argc, char **argv)
{
	uint32_t xorsize;
	char *xor;
	uint32_t num_checksums;
	uint32_t *checksums;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <xor sum> <target file>\n", argv[0]);
		fprintf(stderr, "Warning: It will alter the target file!\n");
		exit(1);
	}

	load_xor_file(argv[1], &xorsize, &xor, &num_checksums, &checksums);

	return 0;
}
