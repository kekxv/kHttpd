//
// Created by caesar on 2019-07-25.
//

#include "RequestData.h"
#include <GetDataParam.h>
#include "regex"
#include "iostream"
#include <event.h>
#include <evhttp.h>

using namespace kHttpdName;

RequestData::RequestData(struct evhttp_request *req) {
    this->req = req;
    GET = new GetDataParam(req);
    HEADER = new GetDataParam(req->input_headers);

    ContentLength = strtoll((*HEADER)["Content-Length"].c_str(), nullptr, 10);
    ContentType = (*HEADER)["Content-Type"];
    string Cookie = (*HEADER)["Cookie"];

    HOST =(*HEADER)["Host"];
    IP = req->remote_host;

    if (!Cookie.empty()) {
        //正则表达式
        string regex_str("([^=]+)=([^;]*);?");
        regex pattern1(regex_str,regex::icase);

        //迭代器声明
        string::const_iterator iter = Cookie.begin();
        string::const_iterator iterEnd= Cookie.end();
        smatch result;
        //正则查找
        while (std::regex_search(iter,iterEnd,result,pattern1))
        {
            string t = result[1];
            string t1 = result[2];
            COOKIE[trim(t)] = trim(t1);
            iter = result[0].second; //更新搜索起始位置
        }
    }

    if (GetMethod() == Method::Post) {
        if (ContentType.find("x-www-form-urlencoded") != string::npos) {
            char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
            POST = new GetDataParam(string("?") + post_data);
        }
    }

    URL = evhttp_request_uri(req);

    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(URL.c_str());
    DecodedUri = decoded_uri;
    free(decoded_uri);

}

RequestData::~RequestData() {
    if (GET != nullptr)delete GET;
    if (HEADER != nullptr)delete HEADER;
    if (POST != nullptr)delete POST;
    GET = nullptr;
    HEADER = nullptr;
}

RequestData::Method RequestData::GetMethod() {
    switch (req->type) {
        case evhttp_cmd_type::EVHTTP_REQ_POST:
            return Method::Post;
        case evhttp_cmd_type::EVHTTP_REQ_HEAD:
            return Method::Head;
        case evhttp_cmd_type::EVHTTP_REQ_PUT:
            return Method::Put;
        case evhttp_cmd_type::EVHTTP_REQ_DELETE:
            return Method::Delete;
        case evhttp_cmd_type::EVHTTP_REQ_OPTIONS:
            return Method::Options;
        case evhttp_cmd_type::EVHTTP_REQ_TRACE:
            return Method::Trace;
        case evhttp_cmd_type::EVHTTP_REQ_GET:
        default:
            return Method::Get;
    }
}
