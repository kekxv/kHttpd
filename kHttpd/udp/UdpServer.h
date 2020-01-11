//
// Created by caesar on 2019-08-17.
//

#ifndef KHTTP_UDPSERVER_H
#define KHTTP_UDPSERVER_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;
struct event;

class UdpServer;
typedef void(*ReadCallback)(UdpServer &udpServer);

class UdpServer {
public:
    static const char *TAG;

    UdpServer(string ip, unsigned int port);

    void SetCallback(ReadCallback readCallback);

    ~UdpServer();

    int Listen();

    size_t read(void *_buf, size_t _bufLen, struct sockaddr_in *client_addr);

    size_t write(void *_buf, size_t _bufLen, struct sockaddr_in *client_addr);

private:
    int sock_fd;
    string IP;
    unsigned int PORT;
    struct event *ev = nullptr;
    ReadCallback cb = nullptr;

    void Init(struct event *ev);

    static void read_cb(int fd, short event, void *arg);
};


#endif //KHTTP_UDPSERVER_H
