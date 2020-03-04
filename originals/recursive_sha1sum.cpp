#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/sha.h>

const int MAX_PATHNAME_LEN = 2048;
const int MAX_FILENAME_LEN = 2048;

const int MAX_HASH_DIGEST = 1024;

struct HashInfo
{
	unsigned char sha1digest[MAX_HASH_DIGEST];
	int sha1digest_len;
};

struct FileInfo
{
	char fullpathname[MAX_PATHNAME_LEN]; 
	char pathname[MAX_PATHNAME_LEN]; // pathname only -- does not include filename
	char filename[MAX_FILENAME_LEN];

	time_t last_modification;
	off_t file_size;

	HashInfo hash_info;
};

std::list <FileInfo> g_fileinfo_list;


// ---------------------------------- begin code ------------------------------------

void recursive_add_files(char *root)
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

			g_fileinfo_list.push_back(finfo);

		}

		else if (S_ISDIR(statbuf.st_mode))
		{
			recursive_add_files(dent->d_name);
		}
	}

	closedir(rootdir);

	chdir(saved_pathname);
}

void analyse(FILE *fpoutput, FileInfo &fileinfo)
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

	SHA_CTX shacontext;

	SHA1_Init(&shacontext);

	while (1)
	{
		r = fread(block, 1, sizeof(block), fp);

		if (r == 0) break;

		SHA1_Update(&shacontext, block, r);

		if (r < sizeof(block))
			break;
	}

	unsigned char sha1digest[1024] = {0};

	SHA1_Final(sha1digest, &shacontext);

	for (int i = 0; i < 20; i++)
	{
		fprintf(fpoutput, "%02x", sha1digest[i]);
	}

	fprintf(fpoutput, " - %s\n", fileinfo.fullpathname);

	fclose(fp);

}

void scan_files(const char *output_filename)
{
	FILE *fp = fopen(output_filename, "wb");
	if (!fp) { perror("fopen"); exit(1); }

	std::list <FileInfo>::iterator it;

	for (it = g_fileinfo_list.begin(); it != g_fileinfo_list.end(); ++it)
	{
		FileInfo &fileinfo = *it; 
		analyse(fp, fileinfo);
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s <output file> <root directory to scan>\n", argv[0]);
		return 0;
	}

	printf("Adding files to list...\n");

	recursive_add_files(argv[2]);

#if 0 // only for debug...
	std::list <FileInfo>::iterator it;

	for (it = g_fileinfo_list.begin(); it != g_fileinfo_list.end(); ++it)
	{
		printf("Filename: %s\n", (*it).fullpathname);
	}

#endif

	scan_files(argv[1]);
}
