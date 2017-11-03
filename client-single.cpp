#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int32_t port;
sockaddr_in server_addr;

void init_server(int argc, char** argv) {
    port = argc < 2 ? 6666 : atoi(argv[1]);
	memset(&server_addr, 0, sizeof server_addr);
    memcpy(&server_addr.sin_addr.s_addr, gethostbyname("localhost")->h_addr, 4);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
}

void send_request(int32_t x) {
	char client_msg[16] = {}, server_msg[16] = {};	
	sprintf(client_msg, "%d", x);
	int32_t client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sockfd < 0)
		throw "Error opening socket";
	if (connect(client_sockfd, (sockaddr*) &server_addr, sizeof server_addr) < 0) 
		throw "Error to connect server";
	write(client_sockfd, client_msg, sizeof client_msg);
	read(client_sockfd, server_msg, sizeof server_msg);
	printf("Client got answer: %s\n", server_msg);
	close(client_sockfd);
}

int main(int32_t argc, char** argv) {
	printf("%d\n", gethostbyname("asff"));
	return 0;
    memcpy(&server_addr.sin_addr.s_addr, gethostbyname("aosnd")->h_addr,
			gethostbyname("aosnd")->h_length);
	int x = server_addr.sin_addr.s_addr;
	printf("%d %d %d %d\n", x & 255, (x >> 8) & 255, (x >> 16) & 255, x >> 24);
	init_server(argc, argv);
	send_request(argc < 3 ? 0 : atoi(argv[2]));
    return 0;
}
