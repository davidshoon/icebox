#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <algorithm>

struct hashline
{
	std::string hash;
	std::string filepath;
};

std::vector <hashline> hashes;

bool compare_hashes(hashline a, hashline b)
{
	return a.hash < b.hash;
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

	std::sort(hashes.begin(), hashes.end(), compare_hashes);

/*
	for (auto &x : hashes) {
		printf("%s %s\n", x.hash.c_str(), x.filepath.c_str());
	}
*/

	for (int i = 0; i < hashes.size() - 1; i++) {
		if (hashes[i].hash == hashes[i+1].hash) {
			printf("%s %s\n", hashes[i].hash.c_str(), hashes[i].filepath.c_str());
			printf("%s %s\n", hashes[i+1].hash.c_str(), hashes[i+1].filepath.c_str());
		}
	}
}
