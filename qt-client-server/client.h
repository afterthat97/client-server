#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <atomic>

#define MAX_THREADS 1024

using std::string;
using std::to_string;
using std::vector;
using std::atomic_bool;

extern vector<string> client_msg;
extern atomic_bool new_message;

class client {
    public:
        client(string, int32_t, int32_t);
        ~client();
        int32_t start();
        void cancel();
        void setHost(string);
        void setPort(int32_t);
        void setNum(int32_t);
        void setX(int32_t);
    private:
        string hostname;
        int32_t port, num;
        vector<pthread_t> tid;
        sockaddr_in server_addr;
        static void* send_request(void*);
};

#endif // CLIENT_H
