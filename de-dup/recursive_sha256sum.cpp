/*

	Recursive SHA256 sum. For outputting to a list for de-duplication of files.

	By David Shoon.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/sha.h>

//#define USE_SHA1

const int MAX_PATHNAME_LEN = 2048;
const int MAX_FILENAME_LEN = 2048;

const int MAX_HASH_DIGEST = 1024;

struct FileInfo
{
	char fullpathname[MAX_PATHNAME_LEN]; 
	char pathname[MAX_PATHNAME_LEN]; // pathname only -- does not include filename
	char filename[MAX_FILENAME_LEN];

	time_t last_modification;
	off_t file_size;
};

static FILE *output_fp{nullptr};

void analyse(FileInfo &fileinfo);

// ---------------------------------- begin code ------------------------------------

void recursive_analyse_files(char *root)
{
	DIR *rootdir;
	char pathname[MAX_PATHNAME_LEN];
	char saved_pathname[MAX_PATHNAME_LEN];

	getcwd(saved_pathname, MAX_PATHNAME_LEN);

	chdir(root);

	getcwd(pathname, MAX_PATHNAME_LEN);

	rootdir = opendir(".");

	struct dirent *dent;

	while ((dent = readdir(rootdir)) != NULL)
	{
		if (strcmp(dent->d_name, ".") == 0)
			continue;

		if (strcmp(dent->d_name, "..") == 0)
			continue;

		struct stat statbuf;

		if (lstat(dent->d_name, &statbuf) == -1)
		{
			perror("lstat");
			printf("dent->d_name: %s\n", dent->d_name);
			continue;
		}

		if (S_ISREG(statbuf.st_mode))
		{
			FileInfo finfo;

			strcpy(finfo.fullpathname, pathname);
			strcat(finfo.fullpathname, "/");
			strcat(finfo.fullpathname, dent->d_name);

			strcpy(finfo.pathname, pathname);
			strcpy(finfo.filename, dent->d_name);

			finfo.last_modification = statbuf.st_mtime;
			finfo.file_size = statbuf.st_size;

			analyse(finfo);
		}

		else if (S_ISDIR(statbuf.st_mode))
		{
			recursive_analyse_files(dent->d_name);
		}
	}

	closedir(rootdir);

	chdir(saved_pathname);
}

void analyse(FileInfo &fileinfo)
{
	FILE *fp;

	fp = fopen(fileinfo.fullpathname, "rb");
	if (!fp)
	{
		perror("fopen");
		printf("%s\n", fileinfo.fullpathname);
		return;
	}

	unsigned char block[1024*1024];
	unsigned int r;

	fseek(fp, 0, SEEK_SET);

#ifdef USE_SHA1
	SHA_CTX shacontext;

	SHA1_Init(&shacontext);
#else

	SHA256_CTX sha256context;
	SHA256_Init(&sha256context);

#endif

	while (1)
	{
		r = fread(block, 1, sizeof(block), fp);

		if (r == 0) break;
#ifdef USE_SHA1
		SHA1_Update(&shacontext, block, r);
#else
		SHA256_Update(&sha256context, block, r);
#endif

		if (r < sizeof(block))
			break;
	}

	unsigned char shadigest[1024] = {0};

#ifdef USE_SHA1
	SHA1_Final(shadigest, &shacontext);
#else
	SHA256_Final(shadigest, &sha256context);
#endif

#ifdef USE_SHA1
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
#else
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
#endif
	{
		fprintf(output_fp, "%02x", shadigest[i]);
	}

	fprintf(output_fp, " %s\n", fileinfo.fullpathname);

	fclose(fp);

}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s <output file> <root directory to scan>\n", argv[0]);
		return 0;
	}

	output_fp = fopen(argv[1], "wb");
	if (!output_fp) { perror("fopen"); exit(1); }

	recursive_analyse_files(argv[2]);
}
