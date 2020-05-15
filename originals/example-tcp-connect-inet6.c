/*

An example of a TCP connection using IPv6.

See also: "man getaddrinfo" on Linux for an alternative way to resolve IPv6 addresses.

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
	struct sockaddr_in6 sin;
	char *string = "Hello World\n";

	if (argc < 3) {
		fprintf(stderr, "Syntax: <hostname> <port>\n");
		exit(1);
	}

	hostname = argv[1];
	port = atoi(argv[2]);

	fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		exit(1);
	}

	sin.sin6_family = AF_INET6;
	sin.sin6_port = htons(port);

	r = inet_pton(AF_INET6, hostname, sin.sin6_addr.s6_addr);

	if (r == 0) {
		fprintf(stderr, "Hostname isn't a valid IPv6 address!\n");
		exit(1);
	}
	else if (r < 0) {
		perror("inet_pton");
		exit(1);
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
