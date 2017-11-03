/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

int32_t port, server_sockfd;
sockaddr_in server_addr;

void init(int argc, char **argv) {
	// Ignore child process's signal
	signal(SIGCHLD, SIG_IGN);
	// if no argument is passed, the default port is 6666
	int port = argc < 2 ? 6666 : atoi(argv[1]);
    // create a socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
		throw "Error when opening socket";
    // clear address structure
    memset(&server_addr, 0, sizeof(server_addr));
    // server byte order
    server_addr.sin_family = AF_INET;  
    // automatically be filled with current host's IP address
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    // convert short integer value for port must be converted into network byte order
    server_addr.sin_port = htons(port);
    // This bind() call will bind the socket to the current IP address on port
    if (bind(server_sockfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
		throw "Error on binding";
    // This listen() call tells the socket to listen to the incoming connections.
    listen(server_sockfd, 100);
	fprintf(stdout, "Server started, listening port %d...\n", port);
}

int32_t f(int32_t x) {
	if (x < 2) return 1;
	else return f(x - 1) + f(x - 2);
}

void worker(int client_sockfd) {
	char client_msg[16] = {}, server_msg[16] = {};
	// This read() function reads the string from the socket
	if (read(client_sockfd, client_msg, sizeof client_msg) < 0)
		throw "Error reading from socket";
	// Calculate f(x) and write to string
	int32_t x;
	sscanf (client_msg, "%d", &x);
	sprintf(server_msg, "%d", f(x));
	// This send() function sends the string to the new socket
    send(client_sockfd, server_msg, strlen(server_msg), 0);
	// Close the socket
	close(client_sockfd);
}

int main(int argc, char **argv) {
	init(argc, argv);
    sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	while (1) {
		// The accept() returns a new socket file descriptor for the connection.
		int client_sockfd = accept(server_sockfd, (sockaddr *) &client_addr, &client_len);
		if (client_sockfd < 0) throw "Error on accept";
		printf("Got connection from %s : %d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		int32_t pid = fork();
		if (pid < 0) { // fork error
			puts("Error on fork");
			close(client_sockfd);
		} else if (pid == 0) { // child process
			worker(client_sockfd);
			break;
		} else // parent process
			close(client_sockfd);
	}
    close(server_sockfd);
    return 0; 
}
