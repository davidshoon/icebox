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
		perror("fread xorsize");
		exit(3);
	}

	// Read number of checksums from file.

	r = fread(num_checksums, sizeof(*num_checksums), 1, fp);
	if (r == 0) {
		perror("fread num_checksums");
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
			perror("fread checksums");
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

	if (fread(*xor, *xorsize, 1, fp) == 0) {
		perror("fread xor");
		exit(6);
	}

	return;
}

void analyse_file(const char *filename, uint32_t xorsize, char *xor, uint32_t num_checksums, uint32_t *checksums)
{
	FILE *fp;
	size_t r;
	char *buf;
	size_t i;
	uint32_t checksum;
	uint32_t checksum_index;
	uint32_t *checksum_errors;
	uint32_t num_actual_errors;

	buf = malloc(xorsize);
	if (!buf) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(7);
	}

	checksum_errors = calloc(num_checksums, sizeof(uint32_t));
	if (!checksum_errors) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(7);
	}

	fp = fopen(filename, "r+");
	if (!fp) {
		perror("fopen");
		exit(7);
	}

	fprintf(stderr, "First pass... analysing checksums...\n");

	checksum_index = 0;
	num_actual_errors = 0;

	while ((r = fread(buf, 1, xorsize, fp))) {
		checksum = 0;
		for (i = 0; i < r; i++) {
			checksum += buf[i];
		}

		checksum_errors[checksum_index] = checksum;

		if (checksum != checksums[checksum_index]) {
			fprintf(stderr, "Found error in block: %d\n", checksum_index);
			num_actual_errors++;

			if (num_actual_errors > 1) {
				char str[1024];

				fprintf(stderr, "We can't fix this file, since number of error blocks exceeds 1 (currently: %d blocks bad).\n", num_actual_errors);
				fprintf(stderr, "Continue scanning anyway? (y/n) ");
				scanf("%s", str);
				if (str[0] != 'y') {
					break;
				}
			}
		}

		if (r < xorsize) {
			break;
		}

		checksum_index++;
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

	analyse_file(argv[2], xorsize, xor, num_checksums, checksums);

	return 0;
}
