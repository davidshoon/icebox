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
#include <openssl/md5.h>
#include <openssl/sha.h>

const int MAX_PATHNAME_LEN = 2048;
const int MAX_FILENAME_LEN = 2048;

const int MAX_HASH_DIGEST = 1024;

struct HeuristicAnalysis
{
	double avg;
	double stddev;
	int min_byte;
	int max_byte;

	// determine if the histogram was uniform
	bool uniform_dist;

	// determine if the histogram wasn't uniform at first, but then became uniform...
	bool moving_uniform_dist;

	// flags used to see if it's "potentially" one of these formats
	bool pot_encrypted;
	bool pot_compressed_archive;
	bool pot_compressed_media;
	bool pot_plaintext;

	// hidden files -- searches to see if it could be an exe... (optional/future feature?)
	bool pot_hidden_executable;
	bool pot_hidden_archive;
	bool pot_hidden_media;
	bool pot_hidden_encrypted;
};

struct HashInfo
{
	unsigned char md5digest[MAX_HASH_DIGEST];
	int md5digest_len;

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

	HeuristicAnalysis heuristic_analysis;

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

void heuristic_analyse(FileInfo &fileinfo)
{
	FILE *fp;

	fp = fopen(fileinfo.fullpathname, "rb");
	if (!fp)
	{
		perror("fopen");
		printf("%s\n", fileinfo.fullpathname);
		return;
	}

	long long total = 0; // total bytes read.
	long long sum = 0; // sum of each byte.
	double avg = 0.0; // avg of each byte
	double stddev = 0.0; // stddev of each byte -- to be done in the 2nd loop
	int min_byte = 0x100;
	int max_byte = 0x000;
	long long histogram[256] = {0}; // histogram of each byte
	double moving_avg = 0.0; // moving average of each byte using 0.9 weighting
	double moving_avg_history[100]; // store moving_avg for each 1 percent of file read
	long long one_percent = (long long) (0.01 * fileinfo.file_size); // approx 1 percent.

	unsigned char block[1024*1024];
	unsigned int r;

	// 1st loop
	while (1)
	{
		r = fread(block, 1, sizeof(block), fp);

		if (r == 0) break;

		for (unsigned int i = 0; i < r; i++)
		{
			if (block[i] > max_byte) max_byte = block[i];
			if (block[i] < min_byte) min_byte = block[i];

			sum += block[i];

			histogram[block[i]]++;

			moving_avg = moving_avg * 0.9 + block[i] * 0.1;

			if ((total + i) % one_percent == 0)
			{
				if ((total + i) / one_percent < 100)
					moving_avg_history[(total + i) / one_percent] = moving_avg;
			}
		}
		

		total += r;

		if (r < sizeof(block))
			break;
	}

	avg = (double) sum / (double) total;

	// 2nd loop -- get stddev

	fseek(fp, 0, SEEK_SET);

	while (1)
	{
		r = fread(block, 1, sizeof(block), fp);

		if (r == 0) break;

		for (unsigned int i = 0; i < r; i++)
		{
			stddev += (block[i] - avg) * (block[i] - avg);
		}
		
		if (r < sizeof(block))
			break;
	}

	stddev = stddev / total;

	stddev = sqrt(stddev);

	// print all the stats out

	printf("======================================\n");
	printf("Filename: %s\n", fileinfo.fullpathname);

	printf("total bytes read: %lld\n", total);
	printf("sum: %lld\n", sum);
	printf("avg: %f\n", avg);
	printf("stddev: %f\n", stddev);
	printf("min_byte: %d\n", min_byte);
	printf("max_byte: %d\n", max_byte);
	printf("histogram: \n");
	for (int i = 0; i < 256; i++)
	{
		printf("%d: %lld\t ", i, histogram[i]);
	}
	printf("\n");

	printf("moving_avg_history: %lld (bytes/interval)\n", one_percent);
	for (int i = 0; i < 100; i++)
	{
		printf("%d: %f\t ", i, moving_avg_history[i]);
	}
	printf("\n");


	// 3rd loop, calculate md5 hash digest using openssl.
	// could probably be merged into 1st or 2nd loop...

	fseek(fp, 0, SEEK_SET);

	MD5_CTX md5context;
	SHA_CTX shacontext;

	MD5_Init(&md5context);
	SHA1_Init(&shacontext);

	while (1)
	{
		r = fread(block, 1, sizeof(block), fp);

		if (r == 0) break;

		MD5_Update(&md5context, block, r);
		SHA1_Update(&shacontext, block, r);

		if (r < sizeof(block))
			break;
	}

	unsigned char md5digest[1024] = {0};
	unsigned char sha1digest[1024] = {0};

	MD5_Final(md5digest, &md5context);
	SHA1_Final(sha1digest, &shacontext);

	printf("MD5 digest: ");

	for (int i = 0; i < 16; i++)
	{
		printf("%02x", md5digest[i]);
	}

	printf("\n");

	printf("SHA1 digest: ");

	for (int i = 0; i < 20; i++)
	{
		printf("%02x", sha1digest[i]);
	}

	printf("\n");

	// 4th -- todo: save all that analysis crap into fileinfo/heuristic/hash etc.

	fclose(fp);

}

void scan_files()
{
	std::list <FileInfo>::iterator it;

	for (it = g_fileinfo_list.begin(); it != g_fileinfo_list.end(); ++it)
	{
		FileInfo &fileinfo = *it; 
		heuristic_analyse(fileinfo);
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s <root directory to scan>\n", argv[0]);
		return 0;
	}

	printf("Adding files to list...\n");

	recursive_add_files(argv[1]);

#if 0 // only for debug...
	std::list <FileInfo>::iterator it;

	for (it = g_fileinfo_list.begin(); it != g_fileinfo_list.end(); ++it)
	{
		printf("Filename: %s\n", (*it).fullpathname);
	}

#endif


	printf("Scanning files with heuristic analyser\n");

	scan_files();

}
