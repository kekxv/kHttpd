//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTPD_KHTTP_H
#define KHTTPD_KHTTP_H

#include <string>
#include <RequestData.h>
#include <ResponseData.h>
#include <vector>

#define HTTPD_SIGNATURE   "httpd v 0.0.1"

struct evbuffer_file_segment;
namespace kHttpdName {
    typedef void (*RouteCallback)(RequestData &, ResponseData &, void *);

    class kHttpd {
    public:
        static const char *TAG;

        /* Try to guess a good content-type for 'path' */
        static const char *GuessContentType(const char *path);

        /**
         * 初始化
         * @param ip 监听地址
         * @param port 监听端口
         * @param timeout 超时时间
         */
        explicit kHttpd(string WebRootPath = ".", int port = 8080, const std::string &ip = "0.0.0.0",
                        int timeout = 120);

        void SetRoute(RouteCallback cb, const char *route = nullptr);

        /**
         * 释放
         */
        ~kHttpd();

        int Listen();

        static int FileType(const string &path);

        static bool isExists(const string &filePath);

        vector<string> defaultIndex{"index.html", "index.htm"};

    private:
        struct evhttp *httpd;
        string WebRootPath = ".";

        //处理模块
        static void httpdHandler(struct evhttp_request *req, void *arg);

        RouteCallback defaultRouteCallback = nullptr;
        map<string, RouteCallback> RouteCallbacks;
        map<string, struct evbuffer_file_segment *> FileEvBufferFiles;
    };

    class kHttpdException : exception {
    public:
        explicit kHttpdException(ResponseData::STATUS status) { this->status = status; }

        ResponseData::STATUS status;
    };
}
#endif //KHTTPD_KHTTP_H
