//
// Created by caesar on 2019-07-29.
//
#include <getopt.h>
#include "kCGI.h"
#include "Log.h"
using namespace kHttpdName;
int main(int argc,char**argv){

    string httpd_option_listen = "127.0.0.1";
    string scriptPath = "127.0.0.1";
    int httpd_option_port = 9000;
    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:vs:")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 's' :
                scriptPath = optarg;
                break;
            case 'p' :
                httpd_option_port = (int)strtol((const char *) optarg, nullptr, 10);
                break;
            case 'v' :
                Log::setConsoleLevel(3);
                break;
            case 'h' :
            default :
                exit(EXIT_SUCCESS);
        }
    }

    try {
        kCGI php(httpd_option_listen,httpd_option_port);
        php.sendStartRequestRecord();
        php.sendParams("SCRIPT_FILENAME",scriptPath.c_str());
        php.sendParams("REQUEST_METHOD","GET");
        php.sendEndRequestRecord();
        map<string,string> header;
        vector<unsigned char> data;
        // kCgiData kData;
        php.ReadFromPhp(header,data);

//        kData.Print();
//        kData.ToVector(data);
        printf("%s",data.data());
    } catch (kCGIException &kException) {
    }
    return 0;
}
