/* 

An example of a TCP connection.

Writes "Hello World" to the socket.

This is designed for Linux, but may work on other unix-like systems.

This is written in ISO C99 -- should be compiled with GCC. 

No comment version

*/

// standard C include files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// unix header files
#include <unistd.h> // for write(), close()

// BSD socket header files
#include <sys/socket.h>
#include <sys/types.h>

// resolver header files
#include <netdb.h>
#include <netinet/in.h>

// hostname to ip address conversion functions
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int fd, r; 
	char *hostname; 
	int port; 
	struct hostent *hent; 
	struct sockaddr_in sin; 
	char *string = "Hello World\n";

	if (argc < 3) {
		fprintf(stderr, "Syntax: <hostname> <port>\n");
		exit(1);
	}

	hostname = argv[1];
	port = atoi(argv[2]);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) { 
		perror("socket");
		exit(1);
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	if (!inet_aton(hostname, &sin.sin_addr)) {
		hent = gethostbyname(hostname);
		if (!hent) {
			perror("gethostbyname");
			exit(1);	
		}

		memcpy(&sin.sin_addr.s_addr, 
			hent->h_addr_list[0], 
			sizeof(sin.sin_addr.s_addr));
	}

	r = connect(fd, (struct sockaddr *) &sin, sizeof(sin));
	if (r < 0) {
		perror("connect");
		exit(1);
	}

	printf("Connected\n");

	write(fd, string, strlen(string));

	close(fd);
}
