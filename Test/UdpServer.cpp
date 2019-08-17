//
// Created by caesar on 2019-08-17.
//

#include <UdpServer.h>
#include <unistd.h>
#include <Log.h>
#include <cerrno>

using namespace std;
using namespace kHttpdName;
#define BUF_SIZE 1024
const char *TAG = "main";

void read_cb(UdpServer &udpServer) {
    unsigned char buf[BUF_SIZE];
    int len;
    struct sockaddr_in client_addr{};
    memset(buf, 0, sizeof(buf));
    len = udpServer.read((void *) buf, sizeof(buf), &client_addr);

    if (len == -1) {
        LogE(TAG, "read error: %d  %s ", errno, strerror(errno));
    } else if (len == 0) {
        LogE(TAG, "Connection Closed");
    } else {
        Log::D_HX("", len, buf, "read");
        udpServer.write(buf, len, &client_addr);
    }
}

void show_help() {
    const char *help = "help (http://kekxv.com)\n\n"
                       "-l <ip_addr>        interface to listen on, default is 0.0.0.0\n"
                       "-p <num>            port number to listen on, default is 10000\n"
                       "-v                  open log show\n"
                       "-h                  print this help and exit\n"
                       "\n";
    fprintf(stderr, "%s", help);
}


int main(int argc, char *argv[]) {
    //默认参数
    string httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 10000;

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:hv")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'v' :
                kHttpdName::Log::setConsoleLevel(3);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }


    UdpServer udpServer(httpd_option_listen, httpd_option_port);
    udpServer.SetCallback(read_cb);
    udpServer.Listen();
    return 0;
}