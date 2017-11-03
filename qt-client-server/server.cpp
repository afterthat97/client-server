#include <server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <signal.h>

using std::to_string;
pthread_mutex_t server_socket_mutex = PTHREAD_MUTEX_INITIALIZER;

int32_t f(int32_t x) {
    if (x < 2) return 1;
    else return f(x - 1) + f(x - 2);
}

std::vector<std::string> server_msg;

server::server(int32_t _port, int32_t _qsize, server_type _stype) {
    port = _port;
    queue_size = _qsize;
    server_sockfd = 0;
    stype = _stype;
    stat = STOPPED;
}

server::~server() {
    if (stat == RUNNING) stop();
}

void server::setPort(int32_t _port) {
    port = _port;
}

void server::setQueue(int32_t _queue) {
    queue_size = _queue;
}

void server::setType(server_type _stype) {
    stype = _stype;
}

void server::start() {
    if (stat == RUNNING) stop();
    signal(SIGCHLD, SIG_IGN);
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        server_msg.push_back("Error on opening socket!");
        return;
    }
    int True = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_sockfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        server_msg.push_back("Error on binding!");
        return;
    }
    listen(server_sockfd, queue_size);
    if (stype == SINGLE) {
        pthread_create(&listener_tid, NULL, listener, &server_sockfd);
        server_msg.push_back("Server(single thread) started, listening on port " + to_string(port));
    } else if (stype == MT) {
        pthread_create(&listener_tid, NULL, listener_mt, &server_sockfd);
        server_msg.push_back("Server(multiple threads) started, listening on port " + to_string(port));
    } else {
        pthread_create(&listener_tid, NULL, listener_mp, &server_sockfd);
        server_msg.push_back("Server(multiple processes) started, listening on port " + to_string(port));
    }

    stat = RUNNING;
}

void server::restart() {
    stop();
    start();
}

void server::stop() {
    if (stat == STOPPED) return;
    pthread_cancel(listener_tid);
    pthread_join(listener_tid, NULL);
    close(server_sockfd);
    server_sockfd = 0;
    server_msg.push_back("Server stopped.");
    stat = STOPPED;
}

void* server::listener(void* server_sockfd) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(sockaddr_in);
    while (1) {
        int client_sockfd = accept(
            *(int*) server_sockfd, (sockaddr *) &client_addr, &client_len);
        if (client_sockfd < 0) {
            server_msg.push_back("Error on accept.");
            continue;
        }
        server_msg.push_back("Got connection from " +
            (std::string)inet_ntoa(client_addr.sin_addr) + ':' + to_string(ntohs(client_addr.sin_port)));
        worker((void*) &client_sockfd);
    }
    return 0;
}

void* server::listener_mt(void * server_sockfd) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int client_sockfd = accept(
            *(int*) server_sockfd, (sockaddr *) &client_addr, &client_len);
        if (client_sockfd < 0) {
            server_msg.push_back("Error on accept.");
            continue;
        }
        server_msg.push_back("Got connection from " +
            (std::string)inet_ntoa(client_addr.sin_addr) + ':' + to_string(ntohs(client_addr.sin_port)));
        pthread_t tid;
        pthread_create(&tid, NULL, worker, new int(client_sockfd));
    }
    return 0;
}

void* server::listener_mp(void * server_sockfd) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int client_sockfd = accept(
            *(int*) server_sockfd, (sockaddr *) &client_addr, &client_len);
        if (client_sockfd < 0) {
            server_msg.push_back("Error on accept.");
            continue;
        }
        server_msg.push_back("Got connection from " +
            (std::string)inet_ntoa(client_addr.sin_addr) + ':' + to_string(ntohs(client_addr.sin_port)));
        int32_t pid = fork();
        if (pid < 0) { // fork error
            server_msg.push_back("Error on fork");
            close(client_sockfd);
        } else if (pid == 0) { // child process
            worker(new int(client_sockfd));
            close(*(int*) server_sockfd);
            exit(0);
        } else // parent process
            close(client_sockfd);
    }
    return 0;
}

void* server::worker(void* client_sockfd) {
    char client_msg[16] = {}, server_msg[16] = {};
    read(*(int*) client_sockfd, client_msg, sizeof client_msg);
    int32_t x = 0;
    sscanf (client_msg, "%d", &x);
    sprintf(server_msg, "%d", f(x));
    send(*(int*) client_sockfd, server_msg, strlen(server_msg), 0);
    pthread_mutex_lock(&server_socket_mutex);
        close(*(int*)client_sockfd);
    pthread_mutex_unlock(&server_socket_mutex);
    return 0;
}
