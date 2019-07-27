#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <string>

#include <kHttpd/H/kHttpd.h>

using namespace std;
using namespace kHttpdName;

void defaultRouteCallback(RequestData &Request, ResponseData &Response, void *){
    string output;

    output += "\nGET";
    output += "\n\turi=" + Request.URL;
    output += "\n\tdecoded_uri=" + Request.DecodedUri;

    output += "\n\tq=" + (*Request.GET)["q"];
    output += "\n\ts=" + (*Request.GET)["s"];

    if (Request.POST != nullptr) {
        output += "\nPOST";
        output += "\n\tHost=" + (*Request.POST)["Host"];
    }
    output += "\nCOOKIE";
    output += "\n\tname=" + Request.COOKIE["name"];
    output += "\n\ts=" + Request.COOKIE["s"];
    output += "\noptions";
    output += "\n\tHOST=" + Request.HOST;
    output += "\n\tip=" + Request.IP;

    Response.PutData(output);
    Response.ContentType = "text/plain; charset=UTF-8";
}
void helloRouteCallback(RequestData &, ResponseData &, void *){

}

void show_help() {
    const char *help = "written by Min (http://54min.com)\n\n"
                       "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
                       "-p <num>     port number to listen on, default is 1984\n"
                       "-d           run as a deamon\n"
                       "-t <second>  timeout for a http request, default is 120 seconds\n"
                       "-h           print this help and exit\n"
                       "\n";
    fprintf(stderr, "%s", help);
}


int main(int argc, char *argv[]) {

    //默认参数
    const char *httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = atoi(optarg);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }

    /* 使用libevent创建HTTP Server */


    kHttpd kHttpd("./");
    kHttpd.SetRoute(defaultRouteCallback);
    kHttpd.SetRoute(helloRouteCallback,"/hello");
    kHttpd.Listen();
    return 0;
}
