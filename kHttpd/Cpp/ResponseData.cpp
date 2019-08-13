//
// Created by caesar on 2019-07-25.
//

#include "kHttpd.h"
#include "ResponseData.h"
#include <Log.h>

namespace kHttpdName {
    /**
     * @brief 获取实际字符串
     *
     * @return string
     */
    string ResponseData::StatusToString(STATUS status) {
        switch (status) {
            case Continue:
                return "Continue";
            case SwitchingProtocols:
                return "Switching Protocols";
            case Processing:
                return "Processing";
            case OK:
                return "OK";
            case Created:
                return "Created";
            case Accepted:
                return "Accepted";
            case NonAuthoritativeInformation:
                return "Non Authoritative Information";
            case NoContent:
                return "No Content";
            case ResetContent:
                return "Reset Content";
            case PartialContent:
                return "Partial Content";
            case MultiStatus:
                return "Multi Status";
            case MultipleChoices:
                return "Multiple Choices";
            case MovedPermanently:
                return "Moved Permanently";
            case MoveTemporarily:
                return "Move Temporarily";
            case SeeOther:
                return "See Other";
            case NotModified:
                return "Not Modified";
            case UseProxy:
                return "Use Proxy";
            case SwitchProxy:
                return "Switch Proxy";
            case TemporaryRedirect:
                return "Temporary Redirect";
            case BadRequest:
                return "Bad Request";
            case Unauthorized:
                return "Unauthorized";
            case PaymentRequired:
                return "Payment Required";
            case Forbidden:
                return "Forbidden";
            case NotFound:
                return "Not Found";
            case MethodNotAllowed:
                return "Method Not Allowed";
            case NotAcceptable:
                return "Not Acceptable";
            case ProxyAuthenticationRequired:
                return "Proxy Authentication Required";
            case RequestTimeout:
                return "Request Timeout";
            case Conflict:
                return "Conflict";
            case Gone:
                return "Gone";
            case LengthRequired:
                return "Length Required";
            case PreconditionFailed:
                return "Precondition Failed";
            case RequestEntityTooLarge:
                return "Request Entity Too Large";
            case RequestURITooLong:
                return "Request URI Too Long";
            case UnsupportedMediaType:
                return "Unsupported Media Type";
            case RequestedRangeNotSatisfiable:
                return "Requested Range Not Satisfiable";
            case ExpectationFailed:
                return "Expectation Failed";
            case TooManyConnections:
                return "Too Many Connections";
            case UnprocessableEntity:
                return "Unprocessable Entity";
            case Locked:
                return "Locked";
            case FailedDependency:
                return "Failed Dependency";
            case UnorderedCollection:
                return "Unordered Collection";
            case UpgradeRequired:
                return "Upgrade Required";
            case RetryWith:
                return "Retry With";
            case UnavailableForLegalReasons:
                return "Unavailable For Legal Reasons";
            case InternalServerError:
                return "Internal Server Error";
            case NotImplemented:
                return "Not Implemented";
            case BadGateway:
                return "Bad Gateway";
            case ServiceUnavailable:
                return "Service Unavailable";
            case GatewayTimeout:
                return "Gateway Timeout";
            case HTTPVersionNotSupported:
                return "HTTP Version Not Supported";
            case VariantAlsoNegotiates:
                return "Variant Also Negotiates";
            case InsufficientStorage:
                return "Insufficient Storage";
            case BandwidthLimitExceeded:
                return "Bandwidth Limit Exceeded";
            case NotExtended:
                return "Not Extended";
            case UnparseableResponseHeaders:
                return "Unparseable Response Headers";
            default:
                return "OK";
        }
    }

    /**
     * @brief Construct a new Http Response Data object
     *
     */
    ResponseData::ResponseData(struct evhttp_request *req,map<string, string> *COOKIE) {
        this->req = req;
        Status = STATUS::OK;
        ContentType = "text/html; charset=UTF-8";
        BodyData = nullptr;
        BodyDataLen = 0;
        type = TYPE::Text;
        FilePath.clear();
        BodyDataText.clear();
        if (COOKIE != nullptr) {
            map<string, string>::iterator iter;
            for (iter = COOKIE->begin(); iter != COOKIE->end(); ++iter) {
                this->COOKIE[iter->first] = iter->second;
            }
        }
        HEADER["Server"] = HTTPD_SIGNATURE;
        HEADER["Connection"] = "close";
    }

    /**
     * @brief Destroy the Http Response Data object
     *
     */
    ResponseData::~ResponseData() {
        ClearBodyData();
    }

    /**
     * @brief 设置返回数据
     *
     * @param data
     */
    void ResponseData::PutData(const string& data) {
        BodyDataText = data;
        type = TYPE::Text;
    }
    /**
     * @brief 设置返回数据
     *
     * @param data
     */
    void ResponseData::PutData(const kHttpdName::JSON &_json) {
        this->json = _json;
        type = TYPE::JSON;
        ContentType = "application/json; charset=utf-8";
    }

    /**
     * @brief 返回文件
     *
     * @param path 文件路径
     */
    void ResponseData::PutFile(const char *path) {
        FilePath = path;
        type = TYPE::File;
    }

    /**
     * @brief 发送数据
     *
     * @param char
     */
    void ResponseData::PutData(unsigned char *Data, long long size) throw(int) {
        ClearBodyData();
        BodyData = new unsigned char[size];
        if (BodyData == nullptr)
            throw -104;
        type = TYPE::Binary;
        memcpy(BodyData, Data, static_cast<size_t>(size));
        BodyDataLen = size;
    }

    /**
     * @brief 清理 BodyData
     *
     */
    void ResponseData::ClearBodyData() {
        if (BodyData != nullptr) {
            delete BodyData;
            BodyData = nullptr;
            BodyDataLen = 0;
        }
    }

    /**
     * @brief Get the Data object 获取文本数据
     *
     * @return string
     */
    string &ResponseData::GetData() throw(int) {
        if (type != TYPE::Text) {
            throw -1;
        }
        return BodyDataText;
    }

    /**
     * @brief Get the Data object 获取文本数据
     *
     * @return string
     */
    JSON ResponseData::GetJSON() throw(int) {
        if (type != TYPE::JSON) {
            throw -1;
        }
        return json;
    }

    /**
     * @brief Get the File object 获取文件路径
     *
     * @return string
     */
    string &ResponseData::GetFile() throw(int) {
        if (type != TYPE::File) {
            throw -1;
        }
        return FilePath;
    }

    /**
     * @brief Get the Data object 获取二进制数据流
     *
     * @param Data
     * @param size
     * @return long long
     */
    long long ResponseData::GetData(unsigned char *Data, long long size) throw(int) {
        if (type != TYPE::Binary) {
            throw -1;
        }
        if (size < BodyDataLen) {
            LogE("HttpServer", "大小错误");
            throw -2;
        }
        if (BodyData != nullptr)
            memcpy(Data, BodyData, BodyDataLen);
        return BodyDataLen;
    }
    /**
     * @brief Get the Data object 获取二进制数据流
     *
     * @param Data
     * @param size
     * @return long long
     */
    unsigned char *ResponseData::GetDataPtr() throw(int) {
        if (type != TYPE::Binary) {
            throw -1;
        }
        return BodyData;
    }

    /**
     * @brief 获取数据大小
     *
     * @return long long
     */
    long long ResponseData::GetByteDataLen() { return BodyDataLen; }

    /**
     * @brief Get the Type object 获取当前类型
     * 
     * @return TYPE 
     */
    ResponseData::TYPE ResponseData::GetType() {
        return type;
    }

}