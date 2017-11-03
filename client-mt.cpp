#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#define MAX_THREADS 1024

int32_t port, client_sockfd[MAX_THREADS];
pthread_t tid[MAX_THREADS];
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
sockaddr_in server_addr;

void init_server(int argc, char** argv) {
    port = argc < 2 ? 6666 : atoi(argv[1]);
	memset(&server_addr, 0, sizeof server_addr);
    memcpy(&server_addr.sin_addr.s_addr, gethostbyname("localhost")->h_addr, 4);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
}

void* send_request(void* arg) {
	int i = *(int*) arg;
	char client_msg[16] = {}, server_msg[16] = {};	
	sprintf(client_msg, "%d", i % 36);

	pthread_mutex_lock(&socket_mutex);
		client_sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (client_sockfd[i] < 0)
			throw "Error opening socket";
		if (connect(client_sockfd[i],
			(sockaddr*) &server_addr, sizeof server_addr) < 0) 
			throw "Error connecting";
	pthread_mutex_unlock(&socket_mutex);

	if (write(client_sockfd[i], client_msg, sizeof client_msg) < 0)
		throw "Error writing to socket";
	if (read(client_sockfd[i], server_msg, sizeof server_msg) < 0)
		throw "Error reading from socket";
	
	pthread_mutex_lock(&socket_mutex);
		printf("Client %d got answer: %s\n", i, server_msg);
		close(client_sockfd[i]);
	pthread_mutex_unlock(&socket_mutex);

	return 0;
}

int main(int32_t argc, char** argv) {
	init_server(argc, argv);
	int32_t tnum = argc < 3 ? MAX_THREADS : atoi(argv[2]);
	for (int32_t i = 0; i < tnum; i++)
		if (pthread_create(&tid[i], NULL, send_request, new int(i)))
			throw "Error creating thread";
	for (int32_t i = 0; i < tnum; i++)
		pthread_join(tid[i], NULL);
    return 0;
}
