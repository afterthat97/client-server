#ifndef SERVER_H
#define SERVER_H

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

extern std::vector<std::string> server_msg;

enum server_status { STOPPED, RUNNING };
enum server_type { SINGLE, MT, MP };

class server {
	public:
        server(int32_t, int32_t, server_type);
		~server();
		void start();
		void restart();
		void stop();
        void setPort(int32_t);
        void setQueue(int32_t);
        void setType(server_type);
	private:
        server_status stat;
        server_type stype;
		int32_t port, queue_size, server_sockfd;
		pthread_t listener_tid;
		sockaddr_in server_addr;
		static void* listener(void*);
        static void* listener_mt(void*);
        static void* listener_mp(void*);
		static void* worker(void*);
};

#endif
