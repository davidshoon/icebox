/* 

	Example TCP server - uses fork()

	Binds and listens on a port, accepts connections and then forks child processes to handle requests.

	By using fork(), this code cannot handle a large amount of simultaneous connections efficiently, but
	keeps the code relatively simple. [1]

	Also, note the use of fdopen() which is POSIX based. This allows stdio functions to be used on
	the socket (e.g. fprintf()). See setvbuf() in man pages to find out how flushing affects your
	communication.


	[1] - Other methods for processing requests would use select or poll so that only a single thread
	would be needed to handle all the sockets.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netdb.h>
#include <netinet/in.h>

#include <arpa/inet.h>

void child(int fd)
{
	FILE *fp_in, *fp_out;

	fp_in = fdopen(fd, "r");
	fp_out = fdopen(fd, "w");

	if (!fp_in || !fp_out) {
		perror("fdopen");
		return;
	}
	
	fprintf(fp_out, "Hello World\n");
	fflush(fp_out);
	sleep(10);


	fclose(fp_in);
	fclose(fp_out);
	close(fd);
	return;
}

int main(int argc, char **argv)
{
	int server_fd;
	int fd;
	int x;
	struct sockaddr_in6 server;
	pid_t pid;
	int port;

	if (argc < 2) {
		printf("Syntax: <listen port>\n");
		exit(1);
	}

	port = atoi(argv[1]);

	server_fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (server_fd < 0) { perror("socket"); exit(1); }

	x = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	server.sin6_family = AF_INET6;
	server.sin6_port = htons(port);
	server.sin6_flowinfo = 0;
	memcpy(server.sin6_addr.s6_addr, in6addr_any.s6_addr, sizeof(server.sin6_addr.s6_addr));
	server.sin6_scope_id = 0;

	if (bind(server_fd, (void *) &server, sizeof(server)) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(server_fd, 5) < 0) { perror("listen"); exit(1); }

	while (1) {
		socklen_t r = sizeof(server);
		fd = accept(server_fd, (void *) &server, &r);
		if (fd < 0) { perror("accept"); exit(1); }

		pid = fork();
		if (pid < 0) { perror("fork"); exit(1); }

		// child
		if (pid == 0) {
			child(fd);
			exit(0);
		}
		// parent
		else {
			close(fd);
			// clear up all the children.
			while (waitpid(-1, NULL, WNOHANG) > 0) ;
		}
	}
}
