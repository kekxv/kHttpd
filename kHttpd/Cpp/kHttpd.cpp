#include <utility>

//
// Created by caesar on 2019-07-25.
//

#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <signal.h>
#include <event.h>
#include <evhttp.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "kHttpd.h"
#include "RequestData.h"
#include "Log.h"


using namespace std;
namespace kHttpdName {

    constexpr static const struct table_entry {
        const char *extension;
        const char *content_type;
    } content_type_table[] = {
            {"txt",   "text/plain"},
            {"c",     "text/plain"},
            {"h",     "text/plain"},
            {"html",  "text/html"},
            {"htm",   "text/htm"},
            {"css",   "text/css"},
            {"js",    "application/javascript"},
            {"json",  "application/json"},
            {"xml",   "application/xml"},
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
                Response.PutFile(filePath.c_str());
                Response.ContentType = GuessContentType(filePath.c_str());
            } else if (fileType == 1) {
                for (const auto &index : that->defaultIndex) {
                    string t = filePath.append("/").append(index);
                    fileType = FileType(t);
                    if (fileType == 2) {
                        Response.PutFile(filePath.c_str());
                        Response.ContentType = GuessContentType(filePath.c_str());
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
                case ResponseData::File: {
                    string filePath = Response.GetFile();
                    if (that->FileEvBufferFiles.find(filePath) == that->FileEvBufferFiles.end()) {
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
                    }

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

            if(Response.Status>=200 && Response.Status<400) {
                LogI(TAG, "请求地址：%s%s %d,%s", Request.HOST.c_str(), Request.URL.c_str(), (int) Response.Status,
                     ResponseData::StatusToString(Response.Status).c_str());
            }else{
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
        return "text/plain";
    }

    int kHttpd::FileType(const string &path) {
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
}
