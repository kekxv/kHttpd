#include <cstdio>
#include <cstdlib>
#include <unistd.h>     //for getopt, fork
#include <cstring>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <string>

#include <kHttpd.h>
#include <Log.h>
#include <CarNumOcr.h>
#include <base64.h>

using namespace std;
using namespace kHttpdName;

CarNumOcr *carNumOcr = nullptr;

void defaultRouteCallback(RequestData &Request, ResponseData &Response, void *) {
    string output;
    if (Request.URL.find("/favicon.ico") != string::npos) {
        Response.Status = ResponseData::STATUS::NotFound;
        Response.PutData("Not Found");
        return;
    }

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

void helloRouteCallback(RequestData &Request, ResponseData &Response, void *) {

}

void carNumOcrCallback(RequestData &Request, ResponseData &Response, void *) {
    JSON json;
    if (carNumOcr == nullptr) {
        Response.Status = ResponseData::STATUS::InternalServerError;
        json.Add(JSON(-1), "error");
        json.Add(JSON("车牌识别未开启"), "message");
        Response.PutData(json);
        return;
    }

    string imgBase64 = Request.json["image"].GetString();
    if (imgBase64.empty()) {
        json.Add(JSON(-1), "error");
        json.Add(JSON("照片错误"), "message");
        Response.PutData(json);
        return;
    }

    size_t len = Base64::DecodedLength(imgBase64.c_str(), imgBase64.size());
    vector<unsigned char> img;
    img.reserve(len);
    Base64::Decode(imgBase64.c_str(), imgBase64.size(), (char *) img.data(), len);
    auto ret = carNumOcr->GetCarNum(img.data(), len);
    JSON jsonArr("[]");
    for (const auto &i : ret) {
        JSON json1;
        json1.Add("Result", JSON(i.second));
        json1.Add("CarNum", JSON(i.first));
        jsonArr.Add(json1);
    }
    json.Add("error", JSON(0));
    json.Add("message", jsonArr);
    Response.PutData(json);

}

void show_help() {
    const char *help = "help (http://kekxv.com)\n\n"
                       "-l <ip_addr>        interface to listen on, default is 0.0.0.0\n"
                       "-p <num>            port number to listen on, default is 1984\n"
                       "-d                  run as a deamon\n"
                       "-t <second>         timeout for a http request, default is 120 seconds\n"
                       "-s <php.sock path>  php sock 模式地址\n"
                       "-C                  车牌识别\n"
                       "-w <web 目录地址>    目录地址\n"
                       "-h                  print this help and exit\n"
                       "\n";
    fprintf(stderr, "%s", help);
}


int main(int argc, char *argv[]) {

    //默认参数
    const char *httpd_option_listen = "0.0.0.0";
    string PhpSockPath;
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds
    string web = "./";

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:ds:t:hvC::w:")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 's' :
                PhpSockPath = optarg;
                break;
            case 'w' :
                web = optarg;
                break;
            case 'p' :
                httpd_option_port = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'v' :
                Log::setConsoleLevel(3);
                break;
            case 'C' :
                if (carNumOcr == nullptr) {
                    if (optarg == nullptr)
                        carNumOcr = new CarNumOcr();
                    else
                        carNumOcr = new CarNumOcr(kHttpd::GetRootPath(optarg));
                }
                LogI("CarNumOcr", "开启车牌识别！");
                break;
            case 'h' :
            default :
                show_help();
                if (carNumOcr != nullptr) {
                    delete carNumOcr;
                    carNumOcr = nullptr;
                }
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
            if (carNumOcr != nullptr) {
                delete carNumOcr;
                carNumOcr = nullptr;
            }
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }

    web = kHttpd::GetRootPath(web);


    kHttpd kHttpd(web.c_str(), httpd_option_port, httpd_option_listen, httpd_option_timeout);
    if (PhpSockPath.empty()) {
        kHttpd.SetCGI("127.0.0.1", 9000);
    } else {
        kHttpd.SetCGI(PhpSockPath);
    }
    kHttpd.SetRoute(defaultRouteCallback);
    kHttpd.SetRoute(helloRouteCallback, "/hello");
    kHttpd.SetRoute(carNumOcrCallback, "/CarNumOcr.json");
    LogI("kHttpdDemo", "服务器开启：http://%s:%d/", httpd_option_listen, httpd_option_port);
    kHttpd.Listen();
    if (carNumOcr != nullptr) {
        delete carNumOcr;
        carNumOcr = nullptr;
    }
    return 0;
}
