#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int evaluate(void *code)
{
	int (*add)() = code;

	if (add(1, 2) == 3) {
		return 0;
	}
	else {
		return -1;
	}
}

void generate_code()
{
	char *buf = mmap(NULL, 1024, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (buf == MAP_FAILED) { perror("mmap"); abort(); }

	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			for (int c = 0; c < 256; c++) {
				for (int d = 0; d < 256; d++) {
					for (int e = 0; e < 256; e++) {
						for (int f = 0; f < 256; f++) {
							for (int g = 0; g < 256; g++) {
								for (int h = 0; h < 256; h++) {
									pid_t child = fork();

									if (child == -1) { perror("fork"); abort(); }
									else if (child) {
										int status;
										wait(&status);

										if (WIFEXITED(status)) {
											exit(0);
										}
										if (WIFSIGNALED(status)) {
											printf("Signal: %d\n", WTERMSIG(status));
										}
									}
									else {
										buf[0] = h;
										buf[1] = g;
										buf[2] = f;
										buf[3] = e;
										buf[4] = d;
										buf[5] = c;
										buf[6] = b;
										buf[7] = a;

										for (int i = 0; i < 8; i++) {
											printf("\\x%x", (unsigned char) buf[i]);
										}
										printf("\n");

										if (evaluate(buf) == 0) {
											printf("Saved\n");
											exit(0);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (munmap(buf, 1024) == -1) { perror("munmap"); abort(); }
}



int main()
{
	generate_code();
}
