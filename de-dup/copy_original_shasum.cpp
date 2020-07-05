/*
	Reading from recursive_sha256sum.cpp's output:

		- Copy original-only files to specified directory (makes sub-directories if sub-directory doesn't exist).

		- Symlink duplicates -- relative to specified directory

	By David Shoon

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

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

void make_path(const std::string &filepathname)
{
	std::string appended_pathname;

	char *pathname = strdup(filepathname.c_str());

	char *p = strrchr(pathname, '/');
	if (p) *p = '\0';

	printf("MAKING PATH: %s\n", pathname);

	char *t = strtok(pathname, "/");

	while (t) {
		appended_pathname += t;
		appended_pathname += "/";

		if (mkdir(appended_pathname.c_str(), 0755) < 0) {
			perror("mkdir"); // errors will occur for "file exists" (i.e. subdirs that already exist) -- this is not an error, but normal, cause I'm too lazy to check if the file exists using stat().
		}
		t = strtok(NULL, "/");

	}

	free(pathname);

}

void copy_file(const char *source_filename, const char *destination_directory)
{
	FILE *fp_in, *fp_out;
	char buf[1024 * 1024]; // 1 meg buffer.
	int total;

	std::string outfilepathname = destination_directory;
	outfilepathname += source_filename;

	printf("COPYING ORIGINAL: %s\n", source_filename);
	printf("TO: %s\n", outfilepathname.c_str());

	make_path(outfilepathname);

	fp_in = fopen(source_filename, "rb");
	fp_out = fopen(outfilepathname.c_str(), "wb");

	if (!fp_in) {
		perror("fopen = rb");
		exit(1);
	}

	if (!fp_out) {
		perror("fopen = wb");
		exit(1);
	}

	while (1) {
		total = fread(buf, 1, sizeof(buf), fp_in);
		int r = fwrite(buf, 1, total, fp_out);

		if (r < total) {
			printf("Error in writing file: %s\n", outfilepathname.c_str());
			exit(1);
		}

		if (total < sizeof(buf)) {
			break;
		}
	}

	fclose(fp_in);
	fclose(fp_out);
}

void make_symlink(const char *original_filename, const char *duplicate_filename, const char *destination_directory)
{
	std::string outfilepathname = destination_directory;
	outfilepathname += duplicate_filename;

	make_path(outfilepathname);

	std::string recursive_dotdot;

	char *filepath = strdup(duplicate_filename);
	char *p;

	while (1) {
		p = strrchr(filepath, '/');
		if (!p)
			break;

		*p = '\0';

		// [2] - This code here is to prevent "one too many ../"
		if (strrchr(filepath, '/') != NULL) {
			recursive_dotdot += "../";	// [1] - previously, this causes one too many "../" relative paths... so we must prevent that...
		}
	}

	recursive_dotdot.pop_back();	// delete last "/" from "../", since original_filename (should) include a leading "/"
	recursive_dotdot += original_filename;

	printf("Making symlink: ORIGINAL [%s], DUPLICATE [%s]\n", recursive_dotdot.c_str(), outfilepathname.c_str());

	if (symlink(recursive_dotdot.c_str(), outfilepathname.c_str()) < 0) {
		perror("symlink");
		exit(1);
	}


	free(filepath);
}

int main(int argc, char **argv)
{
	char buf[4*1024];
	char *arg_hash_filename;
	char *arg_destination_directory;

	if (argc < 3) {
		printf("Usage: %s <hash file name> <destination directory>\n", argv[0]);
		exit(1);
	}

	arg_hash_filename = argv[1];
	arg_destination_directory = argv[2];

	FILE *fp = fopen(arg_hash_filename, "rb");
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

	for (int i = 0; i < hashes.size(); i++) {
		if (matching_hash.hash != hashes[i].hash) {
			matching_hash = hashes[i];

			printf("ORIGINAL %s %s\n", matching_hash.hash.c_str(), matching_hash.filepath.c_str());
			copy_file(matching_hash.filepath.c_str(), arg_destination_directory);
		}

		else {
			printf("DUPLICATE %s %s\n", hashes[i].hash.c_str(), hashes[i].filepath.c_str());
			make_symlink(matching_hash.filepath.c_str(), hashes[i].filepath.c_str(), arg_destination_directory);
		}
	}
}
