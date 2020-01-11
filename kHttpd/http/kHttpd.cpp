#include <utility>

//
// Created by caesar on 2019-07-25.
//

#include <unistd.h>     //for getopt, fork
#include <string>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
#include <evhttp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <kCGI.h>

#include "kHttpd.h"
#include "RequestData.h"
#include "Log.h"


using namespace std;
namespace kHttpdName {
    const char *kHttpd::TAG = "kHttpd";

    kHttpd::kHttpd(string WebRootPath, int port, const std::string &ip, int timeout) {
        this->WebRootPath = std::move(WebRootPath);
        //初始化event API
        event_init();

        //创建一个http server
        httpd = evhttp_start(ip.c_str(), port);
        evhttp_set_timeout(httpd, timeout);

        //指定generic callback
        evhttp_set_gencb(httpd, httpdHandler, this);
        //也可以为特定的URI指定callback
        // evhttp_set_cb(httpd, "/hello", hello_handler, nullptr);

    }

    kHttpd::~kHttpd() {
        for (const auto &i : FileEvBufferFiles) {
            evbuffer_file_segment_free(i.second);
        }
        FileEvBufferFiles.clear();
        event_loopbreak();
        evhttp_free(httpd);
    }

    void kHttpd::httpdHandler(struct evhttp_request *req, void *arg) {
        auto *that = static_cast<kHttpd *>(arg);

        RequestData Request(req);
        ResponseData Response(req, &Request.COOKIE);

        const char *path = evhttp_uri_get_path(evhttp_uri_parse(Request.URL.c_str()));
        if (!path) path = "/";
        string urlPath = path;


        if (that->RouteCallbacks.find(urlPath) != that->RouteCallbacks.end()) {
            that->RouteCallbacks[urlPath](Request, Response, that);
        } else if (isExists(that->WebRootPath + urlPath)) {
            string filePath = that->WebRootPath + urlPath;

            int fileType = FileType(filePath);
            if (fileType == 2) {
                GotoFileType:
                if (filePath.find(".php") == string::npos) {
                    Response.PutFile(filePath.c_str());
                    Response.ContentType = GuessContentType(filePath.c_str());
                } else {
                    if (!that->SockPath.empty()) {
                        try {
                            kCGI php(that->SockPath);
                            // map<string, string> header;
                            vector<unsigned char> data;
                            data.clear();
                            RunPhpCGI(filePath, Request, php, Response.HEADER, data);
                            Response.ContentType = Response.HEADER["Content-type"];
                            Response.HEADER.erase("Content-type");
                            Response.PutData(data.data(), data.size());

                        } catch (kCGIException &kException) {
                            goto GOTO_kCGI;
                        }

                    } else if (!that->IP.empty() && that->PORT > 0) {
                        try {
                            kCGI php(that->IP, that->PORT);
                            // map<string, string> header;
                            vector<unsigned char> data;
                            data.clear();
                            RunPhpCGI(filePath, Request, php, Response.HEADER, data);
                            Response.ContentType = Response.HEADER["Content-type"];
                            Response.HEADER.erase("Content-type");
                            Response.PutData(data.data(), data.size());
                        } catch (kCGIException &kException) {
                            goto GOTO_kCGI;
                        }

                    } else {
                        GOTO_kCGI:
                        Response.HEADER["Content-Type"] = Response.ContentType;
                        struct evbuffer *buf;
                        buf = evbuffer_new();
                        Response.Status = ResponseData::STATUS::InternalServerError;
                        //输出的内容
                        evbuffer_add_printf(buf, "%s", "未开启 PHP 支持");
                        evhttp_send_reply(req, (int) Response.Status,
                                          ResponseData::StatusToString(Response.Status).c_str(), buf);
                        LogE(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                             ResponseData::StatusToString(Response.Status).c_str());
                        evbuffer_free(buf);
                        return;
                    }
                }
            } else if (fileType == 1) {
                for (const auto &index : that->defaultIndex) {
                    string t = filePath;
                    t.append("/").append(index);
                    fileType = FileType(t);
                    if (fileType == 2) {
                        filePath = t;
                        goto GotoFileType;
                        break;
                    }
                }
            }
            if (fileType != 2) {
                that->defaultRouteCallback(Request, Response, that);
            }
        } else if (that->defaultRouteCallback != nullptr) {
            that->defaultRouteCallback(Request, Response, that);
        } else {

        }

        Response.HEADER["Content-Type"] = Response.ContentType;
        /* 输出到客户端 */
        for (const auto &n : Response.HEADER) {
            //HTTP header
            evhttp_add_header(req->output_headers, n.first.c_str(), n.second.c_str());
        }
        //输出的内容
        struct evbuffer *buf;
        buf = evbuffer_new();
        try {
            switch (Response.GetType()) {
                case ResponseData::Text: {
                    evbuffer_add_printf(buf, "%s", Response.GetData().c_str());
                    evhttp_send_reply(req, (int) Response.Status, ResponseData::StatusToString(Response.Status).c_str(),
                                      buf);

                }
                    break;
                case ResponseData::JSON: {
                    evbuffer_add_printf(buf, "%s", Response.GetJSON().ToString().c_str());
                    evhttp_send_reply(req, (int) Response.Status, ResponseData::StatusToString(Response.Status).c_str(),
                                      buf);

                }
                    break;
                case ResponseData::File: {
                    string filePath = Response.GetFile();
                    if (that->FileEvBufferFiles.find(filePath) != that->FileEvBufferFiles.end()) {
                        auto _ = that->FileEvBufferFiles[filePath];
                        evbuffer_file_segment_free(_);
                    }
                    int fileType = FileType(filePath);
                    int fd = open(filePath.c_str(), O_RDWR);
                    if (fd == -1) {
                        throw kHttpdException(ResponseData::NotFound);
                    }

                    struct evbuffer_file_segment *seg = evbuffer_file_segment_new(fd, 0, -1, 0);
                    if (seg == nullptr) {
                        throw kHttpdException(ResponseData::InternalServerError);
                    }
                    that->FileEvBufferFiles[filePath] = seg;

                    evbuffer_add_file_segment(buf, that->FileEvBufferFiles[filePath], 0, -1);
                    Response.Status = ResponseData::OK;
                    evhttp_send_reply(req, (int) Response.Status,
                                      ResponseData::StatusToString(Response.Status).c_str(), buf);
                }
                    break;
                case ResponseData::Binary: {
                    evbuffer_add(buf, Response.GetDataPtr(), Response.GetByteDataLen());
                    evhttp_send_reply(req, (int) Response.Status,
                                      ResponseData::StatusToString(Response.Status).c_str(), buf);
                }
                    break;
            }

            if (Response.Status >= 200 && Response.Status < 400) {
                LogI(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                     ResponseData::StatusToString(Response.Status).c_str());
            } else {
                LogE(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                     ResponseData::StatusToString(Response.Status).c_str());
            }

        } catch (kHttpdException &err) {
            Response.Status = err.status;
            Response.PutData(ResponseData::StatusToString(Response.Status));
            //输出的内容
            evbuffer_add_printf(buf, "%s", Response.GetData().c_str());
            evhttp_send_reply(req, (int) Response.Status,
                              ResponseData::StatusToString(Response.Status).c_str(), buf);
            LogE(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                 ResponseData::StatusToString(Response.Status).c_str());
        } catch (...) {
            //输出的内容
            evbuffer_add_printf(buf, "%s", "Page not found");
            Response.Status = ResponseData::NotFound;
            evhttp_send_reply(req, (int) Response.Status, ResponseData::StatusToString(Response.Status).c_str(), buf);

            LogE(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                 ResponseData::StatusToString(Response.Status).c_str());
        }
        evbuffer_free(buf);
    }

    int kHttpd::Listen() {
        //循环处理events
        return event_dispatch();
    }

    void kHttpd::SetRoute(RouteCallback cb, const char *route) {
        if (route == nullptr) {
            defaultRouteCallback = cb;
            return;
        }
        RouteCallbacks[route] = cb;
    }

    bool kHttpd::isExists(const string &filePath) {
        if (FILE *file = fopen(filePath.c_str(), "r")) {
            fclose(file);
            return true;
        } else {
            return false;
        }
    }

    const char *kHttpd::GuessContentType(const char *path) {
        const char *last_period, *extension;
        const struct table_entry *ent;
        last_period = strrchr(path, '.');
        if (!last_period || strchr(last_period, '/'))
            goto not_found; /* no exension */
        extension = last_period + 1;
        for (ent = &content_type_table[0]; ent->extension; ++ent) {
            if (!evutil_ascii_strcasecmp(ent->extension, extension))
                return ent->content_type;
        }

        not_found:
        return "text/plain; charset=utf-8";
    }

    int kHttpd::FileType(const string &path) {
        //LogD(TAG,"%s",path.c_str());
        struct stat s{};
        if (0 == stat(path.c_str(), &s)) {
            if (s.st_mode & S_IFDIR) {
                //                cout << "DIR" << endl;
                return 1;
            } else if (s.st_mode & S_IFREG) {
                //                cout << "FILE" << endl;
                return 2;
            } else {
                //                cout << "?" << endl;
                return 0;
            }
        } else {
            //            cout << "ERR" << endl;
        }
        return -1;
    }

    void kHttpd::SetCGI(string sockPath) {
        this->SockPath = std::move(sockPath);
    }

    void kHttpd::SetCGI(string ip, int port) {
        this->IP = std::move(ip);
        this->PORT = port;
    }

    void kHttpd::RunPhpCGI(const string &filePath, RequestData &Request, kCGI &kCgi,
                           map<string, string> &header,
                           vector<unsigned char> &data) {
        kCgi.sendStartRequestRecord();
        kCgi.sendParams("SCRIPT_FILENAME", filePath.c_str());
        kCgi.sendParams("REQUEST_METHOD", Request.GetMethod() == RequestData::Method::Post ? "POST" : "GET");
        kCgi.sendParams("REMOTE_HOST", Request.IP.c_str());
        kCgi.sendParams("SERVER_NAME", Request.HOST.c_str());
        kCgi.sendParams("SERVER_SOFTWARE", HTTPD_SIGNATURE);
        kCgi.sendParams("HTTP_COOKIE", Request.COOKIES.c_str());
        kCgi.sendEndRequestRecord();
//        kCgiData kData;
        kCgi.ReadFromPhp(header, data);
//        kData.ToVector(data);
    }
}
