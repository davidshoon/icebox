#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	FILE *fp_in, *fp_out;
	long offset;
	char buf[1024];
	size_t r;

	if (argc < 4) {
		fprintf(stderr, "Usage: %s <target filename> <patch file> <patch offset>\n", argv[0]);
		exit(1);
	}

	offset = atol(argv[3]);

	printf("Patching at offset: %ld -- is this correct? (y/n) ", offset);
	buf[0] = 'n';
	if ((scanf("%s", buf) != 1) || (buf[0] != 'y')) {
		exit(1);
	}

	fp_in = fopen(argv[2], "rb");
	fp_out = fopen(argv[1], "r+");

	fseek(fp_out, offset, SEEK_SET);

	while ((r = fread(buf, 1, sizeof(buf), fp_in))) {
		fwrite(buf, 1, r, fp_out);

		if (r < sizeof(buf)) {
			break;
		}
	}

	fclose(fp_in);
	fclose(fp_out);
}
