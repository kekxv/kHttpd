//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTP_REQUESTDATA_H
#define KHTTP_REQUESTDATA_H

#include <string>
#include <map>
#include <GetDataParam.h>

using namespace std;

namespace kHttpdName {
    class RequestData {
    public:
        enum Method {
            Options,
            Get,
            Head,
            Post,
            Put,
            Delete,
            Trace
        };

        /**
          * @brief Construct a new Http Request Data object
          *
          */
        explicit RequestData(struct evhttp_request *req);

        /**
          * @brief Destroy the Http Request Data object
          *
          */
        ~RequestData();

        /**
          * @brief Get the Method object 获取当前请求类型
          *
          * @return Method
          */
        Method GetMethod();

        /**
          * @brief GET 参数
          *
          */
        GetDataParam *GET = nullptr;

        /**
          * @brief POST 参数
          *
          */
        GetDataParam *POST = nullptr;
        /**
          * @brief COOKIE 参数
          *
          */
        map<string,string> COOKIE;
        /**
          * @brief COOKIE 参数
          *
          */
        string COOKIES;
        /**
          * @brief HEADER 参数
          *
          */
        GetDataParam *HEADER = nullptr;

        /**
          * @brief 请求的地址
          *
          */
        string URL;
        /**
          * @brief 请求的地址
          *
          */
        string DecodedUri;
        /**
          * @brief 域名
          *
          */
        string HOST;
        /**
          * @brief IP
          *
          */
        string IP;

        /**
          * @brief 类型
          *
          */
        string ContentType;
        /**
          * @brief 数据大小
          *
          */
        long long ContentLength = 0;
        /**
          * @brief 最大数据大小
          *
          */
        const size_t MaxContentLength = 256 * 1024 * 1024;

        unsigned char* GetBodyData();

    private:
        struct evhttp_request *req;
    };
}


#endif //KHTTP_REQUESTDATA_H
