/* 

	Example TCP server - uses select(), doesn't fork() or use threads. More efficient.

	NB: 
	It's optional if you want to use non-blocking sockets --
	the benefit is only during send() as it doesn't block and returns the amount sent.

	Reading doesn't block cause you're using select to detect whether there's data,
	and read() returns as much data as possible if there is data, otherwise if there's no data,
	read blocks (but this will never happen with our select loop).

	This is basically how various daemons in UNIX are programmed to handle multiple connections
	with a single thread/process. 

	I use C++ here since it's easier to handle a vector of client structs, but you could potentially
	use only C.

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

#include <sys/select.h>


#include <vector>
#include <string>
#include <algorithm>

#include <time.h>

struct Client
{
	int fd;
	struct sockaddr_in address;

	std::vector <char> in_buffer;
	std::vector <char> out_buffer;

	void read()
	{
		printf("Client::read...\n"); fflush(stdout);

		std::string str;

		for (int i = 0; i < in_buffer.size(); i++) {
			if (in_buffer[i] == '\n') {
				printf("Found newline\n"); fflush(stdout);

				for (int j = 0; j < i; j++) {
					str.push_back(in_buffer[j]);
				}

				in_buffer.erase(in_buffer.begin(), in_buffer.begin() + str.size() + 1);

				break;
			}
		}

		if (str.size() > 0) {
			printf("Client received: %s\n", str.c_str());

			// echo back to sender:

			for (int i = 0; i < str.size(); i++) {
				out_buffer.push_back(in_buffer[i]);
			}
		}
	}
};

char *strip_newline(char *s)
{
	char *p = strpbrk(s, "\r\n");
	if (p) *p = '\0';
	return s;
}

int main(int argc, char **argv)
{
	int server_fd;
	int fd;
	int x;
	struct sockaddr_in server;
	pid_t pid;
	int port;

	if (argc < 2) {
		printf("Syntax: <listen port>\n");
		exit(1);
	}

	port = atoi(argv[1]);

	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) { perror("socket"); exit(1); }

	x = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	if (bind(server_fd, (sockaddr *) &server, sizeof(server)) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(server_fd, 5) < 0) { perror("listen"); exit(1); }


	std::vector <Client> clients;

	while (1) {
		fd_set readfds, writefds;
		struct timeval timeout;
		int maxfds = -1;

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		maxfds = std::max(server_fd, maxfds);
		FD_SET(server_fd, &readfds);

		for (auto &client : clients) {
			if (client.fd != -1) {
				maxfds = std::max(client.fd, maxfds);
				FD_SET(client.fd, &readfds);

				if (client.out_buffer.size() > 0) {
					FD_SET(client.fd, &writefds);
				}
			}
		}

		int r = select(maxfds + 1, &readfds, &writefds, NULL, &timeout);

		if (r == 0) {
			printf("select timed out, nothing happened\n"); fflush(stdout);
			continue;
		}
		else if (r == -1) {
			perror("select");
			continue;
		}

		if (FD_ISSET(server_fd, &readfds)) {
			printf("Accepting connections...\n"); fflush(stdout);

			socklen_t r = sizeof(server);
			fd = accept(server_fd, (sockaddr *) &server, &r);
			if (fd < 0) { perror("accept"); exit(1); }

			time_t now = time(NULL);

			printf("Connected IP: %s [%s]\n", inet_ntoa(server.sin_addr), strip_newline(ctime(&now))); fflush(stdout);

			struct Client client;
			client.fd = fd;
			client.address = server;
			clients.push_back(client);
		}


		printf("Checking for client input/output...\n "); fflush(stdout);
		for (auto &client : clients) {
			printf("There are clients to check...................\n"); fflush(stdout);

			if (FD_ISSET(client.fd, &readfds)) {
				printf("Client input...\n"); fflush(stdout);

				char buf[1024];
				int total = recv(client.fd, buf, sizeof(buf), 0);

				if (total == -1) { 
					perror("recv");
					printf("Closing socket...\n");
					close(client.fd);
					client.fd = -1;
					continue;
				}

				if (total == 0) {
					printf("Closing socket...\n");
					close(client.fd);
					client.fd = -1;
					continue;
				}

				for (int i = 0; i < total; i++) {
					client.in_buffer.push_back(buf[i]);
				}

				client.read();
			}

			if (FD_ISSET(client.fd, &writefds)) {
				printf("Client output...\n"); fflush(stdout);

				int total = send(client.fd, client.out_buffer.data(), client.out_buffer.size(), MSG_NOSIGNAL);

				if (total == -1) {
					perror("send");
					printf("Closing socket...\n");
					close(client.fd);
					client.fd = -1;
				}

				else {
					client.out_buffer.erase(client.out_buffer.begin(), client.out_buffer.begin() + total);
				}
			}
		}

		// delete the clients with closed sockets.

		auto end_it = std::remove_if(clients.begin(), clients.end(), [] (auto &client) { printf("client.fd == %d\n", client.fd); return (client.fd == -1); });
		clients.erase(end_it, clients.end());


	}
}
