/*
	List duplicate-only files from recursive_sha256sum.cpp's output.

	By David Shoon

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <algorithm>

#include <unistd.h>

struct hashline
{
	std::string hash;
	std::string filepath;
};

std::vector <hashline> hashes;

bool compare_hashlines(hashline a, hashline b)
{
	return (a.hash + a.filepath) < (b.hash + b.filepath);
}

char *strip_newline(char *s)
{
	char *p = strpbrk(s, "\r\n");
	if (p) *p = '\0';
	return s;
}

int main(int argc, char **argv)
{
	char buf[4*1024];

	if (argc < 2) {
		printf("Usage: %s <hash file name>\n", argv[0]);
		exit(1);
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) { perror("fopen"); exit(1); }

	while (fgets(buf, sizeof(buf), fp)) {
		strip_newline(buf);

		char *tok = strtok(buf, " ");
		hashline line;
		line.hash = tok;

		tok = strtok(NULL, "");
		line.filepath = tok;

		hashes.push_back(line);
	}

	std::sort(hashes.begin(), hashes.end(), compare_hashlines);

/*
	for (auto &x : hashes) {
		printf("%s %s\n", x.hash.c_str(), x.filepath.c_str());
	}
*/

	hashline matching_hash = { "", "" };
	bool first_match = false;

	for (int i = 0; i < hashes.size(); i++) {
		if (matching_hash.hash != hashes[i].hash) {
			matching_hash = hashes[i];
			first_match = true;
		}

		else {
			if (first_match) {
				printf("ORIGINAL %s %s\n", matching_hash.hash.c_str(), matching_hash.filepath.c_str());
				first_match = false;
			}

			printf("COPY %s %s\n", hashes[i].hash.c_str(), hashes[i].filepath.c_str());
		}
	}
}
