//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTP_RESPONSEDATA_H
#define KHTTP_RESPONSEDATA_H

#include <string>
#include <map>
#include <CJsonObject.hpp>
using namespace std;

namespace kHttpdName {
    class ResponseData {
    public:
        /**
          * @brief Http 状态码
          *
          */
        enum STATUS
        {
            Continue = 100,
            SwitchingProtocols,
            Processing,
            OK = 200,
            Created = 201,
            Accepted = 202,
            NonAuthoritativeInformation = 203,
            NoContent = 204,
            ResetContent = 205,
            PartialContent = 206,
            MultiStatus = 207,
            MultipleChoices = 300,
            MovedPermanently = 301,
            MoveTemporarily = 302,
            SeeOther = 303,
            NotModified = 304,
            UseProxy = 305,
            SwitchProxy = 306,
            TemporaryRedirect = 307,
            BadRequest = 400,
            Unauthorized = 401,
            PaymentRequired = 402,
            Forbidden = 403,
            NotFound,
            MethodNotAllowed,
            NotAcceptable,
            ProxyAuthenticationRequired,
            RequestTimeout,
            Conflict,
            Gone,
            LengthRequired,
            PreconditionFailed,
            RequestEntityTooLarge,
            RequestURITooLong,
            UnsupportedMediaType,
            RequestedRangeNotSatisfiable,
            ExpectationFailed,
            TooManyConnections = 421,
            UnprocessableEntity,
            Locked,
            FailedDependency,
            UnorderedCollection,
            UpgradeRequired,
            RetryWith = 449,
            UnavailableForLegalReasons = 451,
            InternalServerError = 500,
            NotImplemented,
            BadGateway,
            ServiceUnavailable,
            GatewayTimeout,
            HTTPVersionNotSupported,
            VariantAlsoNegotiates,
            InsufficientStorage,
            BandwidthLimitExceeded = 509,
            NotExtended,
            UnparseableResponseHeaders = 600,
        };
        /**
         * @brief 获取实际字符串
         *
         * @return string
         */
        static string StatusToString(STATUS status);
        /**
         * @brief 返回的类型
         *
         */
        enum TYPE
        {
            /**
             * @brief 文件
             *
             */
                    File,
            /**
             * @brief 文本
             *
             */
                    Text,
            /**
             * @brief 二进制流
             *
             */
                    Binary,
            /**
             * @brief 文本
             *
             */
                    JSON,
        };
        /**
          * @brief 设置返回的状态码
          *
          */
        STATUS Status;
        /**
          * @brief 类型
          *
          */
        string ContentType = "text/html; charset=UTF-8";
        /**
         * @brief Construct a new Http Response Data object
         *
         */
        explicit ResponseData(struct evhttp_request *req,map<string, string> *COOKIE = nullptr);
        /**
         * @brief Destroy the Http Response Data object
         *
         */
        ~ResponseData();
        /**
          * @brief 设置返回数据
          *
          * @param data
          */
        void PutData(const string& data);
        /**
         * @brief 设置返回数据
         *
         * @param data
         */
        void PutData(const CJsonObject &_json);
        /**
          * @brief 发送数据
          *
          * @param char
          */
        void PutData(unsigned char *Data, long long size) throw(int);
        /**
          * @brief 返回文件
          *
          * @param path 文件路径
          */
        void PutFile(const char *path);
        /**
         * @brief Get the Data object 获取文本数据
         *
         * @return string
         */
        string &GetData() throw(int);
        CJsonObject GetJSON() throw(int);
        /**
         * @brief Get the File object 获取文件路径
         *
         * @return string
         */
        string &GetFile() throw(int);
        /**
         * @brief Get the Data object 获取二进制数据流
         *
         * @param Data
         * @param size
         * @return long long
         */
        long long GetData(unsigned char *Data, long long size) throw(int);
        /**
         * @brief Get the Data object 获取二进制数据流
         *
         * @param Data
         * @param size
         * @return long long
         */
        unsigned char *GetDataPtr() throw(int);
        /**
         * @brief 获取数据大小
         *
         * @return long long
         */
        long long GetByteDataLen();
        /**
          * @brief COOKIE 参数
          *
          */
        map<string, string> COOKIE;
        /**
          * @brief HEADER 参数
          *
          */
        map<string, string> HEADER;

        /**
         * @brief Get the Type object 获取当前类型
         *
         * @return TYPE
         */
        TYPE GetType();

    private:
        /**
          * @brief 数据
          *
          */
        unsigned char *BodyData;
        /**
         * @brief 数据大小
         *
         */
        long long BodyDataLen;
        /**
         * @brief 清理 BodyData
         *
         */
        void ClearBodyData();
        /**
         * @brief 返回文件地址
         *
         */
        string FilePath;
        /**
         * @brief 文本数据
         *
         */
        string BodyDataText;

        CJsonObject json;

        /**
         * @brief 类型
         *
         */
        TYPE type;

        struct evhttp_request *req = nullptr;
    };
}

#endif //KHTTP_RESPONSEDATA_H
