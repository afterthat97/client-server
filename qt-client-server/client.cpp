#include "client.h"

pthread_mutex_t client_socket_mutex = PTHREAD_MUTEX_INITIALIZER;

std::vector<string> client_msg;
std::atomic_bool new_message;
std::atomic_int32_t client_id;
int32_t x = -1;

client::client(string _host, int32_t _port, int32_t _num) {
    hostname = _host; port = _port; num = _num;
    tid.resize(num + 1);
}

client::~client() {
    cancel();
}

int32_t client::start() {
    memset(&server_addr, 0, sizeof server_addr);
    hostent* host = gethostbyname(hostname.c_str());
    if (host == NULL) return 0;
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    client_id = 0;

    pthread_mutex_lock(&client_socket_mutex);
    client_msg.clear();
    client_msg.resize(num + 1);
    client_msg[0] = "Start " + to_string(num) + " clients...";
    pthread_mutex_unlock(&client_socket_mutex);

    for (int32_t i = 1; i <= num; i++)
        if (pthread_create(&tid[i], NULL, send_request, &server_addr)) {
            new_message = 1;
            pthread_mutex_lock(&client_socket_mutex);
            client_msg[i] = ("Client " + to_string(i) + " failed to create thread!");
            pthread_mutex_unlock(&client_socket_mutex);
        } else {
            new_message = 1;
            pthread_mutex_lock(&client_socket_mutex);
            client_msg[i] = ("Client " + to_string(i) +
                " requesting " + hostname + ':' + to_string(port) + ", ");
            pthread_mutex_unlock(&client_socket_mutex);
        }
    return 1;
}

void client::cancel() {
    for (int32_t i = 0; i < num; i++) {
        pthread_cancel(tid[i]);
        pthread_join(tid[i], NULL);
    }
}

void client::setHost(std::string _host) {
    hostname = _host;
}

void client::setNum(int32_t _num) {
    num = _num;
    tid.resize(num + 1);
}

void client::setPort(int32_t _port) {
    port = _port;
}

void client::setX(int32_t _x) {
    x = _x;
}

void* client::send_request(void* arg) {
    char val[16] = {}, ans[16] = {};
    sprintf(val, "%d", x == -1 ? rand() % 40 : x);

    pthread_mutex_lock(&client_socket_mutex);
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0), i = ++client_id, True = 1;
    if (client_sockfd < 0) {
        new_message = 1;
        client_msg[i] = ("Client " + to_string(i) + " failed to open socket!");
        pthread_mutex_unlock(&client_socket_mutex);
        return 0;
    }
    setsockopt(client_sockfd, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int));
    if (connect(client_sockfd, (sockaddr*) arg, sizeof server_addr) < 0) {
        new_message = 1;
        client_msg[i] = ("Client " + to_string(i) + " failed to connect!");
        pthread_mutex_unlock(&client_socket_mutex);
        close(client_sockfd);
        return 0;
    }
    client_msg[i] += ("x = " + string(val) +"...");
    pthread_mutex_unlock(&client_socket_mutex);

    bool fail = (write(client_sockfd, val, sizeof val) < 0
                || read(client_sockfd, ans, sizeof ans) < 0);

    pthread_mutex_lock(&client_socket_mutex);
    if (fail) {
        client_msg[i] += ("error");
    } else {
        client_msg[i] += ("f(x) = " + string(ans));
    }
    new_message = 1;
    close(client_sockfd);
    pthread_mutex_unlock(&client_socket_mutex);
    return 0;
}
