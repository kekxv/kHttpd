//
// Created by caesar on 2019-08-17.
//

#include "UdpServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>
#include <Log.h>

using namespace std;

const char *UdpServer::TAG = "UdpServer";

UdpServer::UdpServer(string ip, unsigned int port) {
    /* Init. event */
    if (event_init() == nullptr) {
        LogE(TAG, "event_init() failed");
        return;
    }
    ev = new struct event();
    this->IP = std::move(ip);
    this->PORT = port;
    Init(ev);
}

UdpServer::~UdpServer() {
    if (ev != nullptr) {
        delete ev;
    }
    ev = nullptr;
    close(sock_fd);
}

int UdpServer::Listen() {
    return event_dispatch();
}

void UdpServer::Init(struct event *ev) {
    int flag = 1;
    struct sockaddr_in sin{};

/* Create endpoint */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LogE(TAG, "socket error: %d  %s ", errno, strerror(errno));
        return;
    }

    /* Set socket option */
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) {
        LogE(TAG, "setsockopt error: %d  %s ", errno, strerror(errno));
        return;
    }

    /* Set IP, port */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(IP.c_str());
    sin.sin_port = htons(PORT);

    /* Bind */
    if (::bind(sock_fd, (struct sockaddr *) &sin, sizeof(struct sockaddr)) < 0) {
        LogE(TAG, "bind error: %d  %s ", errno, strerror(errno));
        return;
    } else {
        LogI(TAG, "bind success: [%s] [%u] ", IP.c_str(), PORT);
    }

    /* Init one event and add to active events */
    event_set(ev, sock_fd, EV_READ | EV_PERSIST, &read_cb, this);
    if (event_add(ev, nullptr) == -1) {
        LogE(TAG, "event_add error: %d  %s ", errno, strerror(errno));
    }
}

size_t UdpServer::write(void *_buf, size_t _bufLen, struct sockaddr_in *client_addr) {
    unsigned int size = sizeof(struct sockaddr);
    return sendto(sock_fd, _buf, _bufLen, 0, (struct sockaddr *) client_addr, size);
}

size_t UdpServer::read(void *_buf, size_t _bufLen, struct sockaddr_in *client_addr) {
    unsigned int size = sizeof(struct sockaddr);
    return recvfrom(sock_fd, (void *) _buf, _bufLen, 0, (struct sockaddr *) client_addr, &size);
}

void UdpServer::read_cb(int fd, short event, void *arg) {
    auto that = (UdpServer *) arg;
    that->cb(*that);
}

void UdpServer::SetCallback(ReadCallback readCallback) {
    this->cb = readCallback;
}
