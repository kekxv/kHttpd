//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTPD_KHTTP_H
#define KHTTPD_KHTTP_H

#include <string>
#include <RequestData.h>
#include <ResponseData.h>
#include <vector>
#include "kCGI.h"

#define HTTPD_SIGNATURE   "httpd v 0.0.1"

struct evbuffer_file_segment;
namespace kHttpdName {
    typedef void (*RouteCallback)(RequestData &, ResponseData &, void *);

    constexpr static const struct table_entry {
        const char *extension;
        const char *content_type;
    } content_type_table[] = {
            {"txt",   "text/plain; charset=utf-8"},
            {"c",     "text/plain; charset=utf-8"},
            {"h",     "text/plain; charset=utf-8"},
            {"php",   "text/html; charset=utf-8"},
            {"html",  "text/html; charset=utf-8"},
            {"htm",   "text/htm; charset=utf-8"},
            {"css",   "text/css"},
            {"js",    "application/javascript; charset=utf-8"},
            {"json",  "application/json; charset=utf-8"},
            {"xml",   "application/xml; charset=utf-8"},
            {"gif",   "image/gif"},
            {"jpg",   "image/jpeg"},
            {"jpeg",  "image/jpeg"},
            {"png",   "image/png"},
            {"bmp",   "image/bmp"},
            {"woff",  "application/font-woff"},
            {"ico",   "image/x-icon"},
            {"pdf",   "application/pdf"},
            {"ps",    "application/postscript"},
            {nullptr, nullptr},
    };


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

        vector<string> defaultIndex{"index.php", "index.html", "index.htm"};

        void SetCGI(string ip, int port);

        void SetCGI(string sockPath);

    private:
        struct evhttp *httpd;
        string WebRootPath = ".";
        string SockPath;
        string IP = "";
        int PORT = 0;

        //处理模块
        static void httpdHandler(struct evhttp_request *req, void *arg);

        static void RunPhpCGI(const string& filePath,RequestData &Request,kCGI &kCgi,
                              map<string, string> &header,
                              vector<unsigned char> &data);

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
