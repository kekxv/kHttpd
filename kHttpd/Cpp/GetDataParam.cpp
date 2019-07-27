//
// Created by caesar on 2019-07-26.
//

#include <event.h>
#include <evhttp.h>
#include "GetDataParam.h"
using namespace kHttpdName;

GetDataParam::GetDataParam(struct ::evhttp_request *req) {
    if (this->params == nullptr) { this->params = new struct::evkeyvalq(); }
    const char *uri;
    string decodedUri;

    uri = evhttp_request_uri(req);

    //解析URI的参数(即GET方法的参数)
    evhttp_parse_query(uri, params);
}

string GetDataParam::operator[](const string &key) {
    const char *val = evhttp_find_header(params, key.c_str());
    if (val == nullptr)return "";
    string value = DecodeURI(val);
    return value;
}

GetDataParam::GetDataParam(const string &keyValueString) {
    if (this->params == nullptr) { this->params = new struct::evkeyvalq(); }
    //解析URI的参数(即GET方法的参数)
    evhttp_parse_query(keyValueString.c_str(), params);
}

GetDataParam::GetDataParam(struct ::evkeyvalq *params) {
    this->flag = false;
    this->params = params;
}

string GetDataParam::DecodeURI(const string &url) {
    string data;
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(url.c_str());
    data = decoded_uri;
    free(decoded_uri);
    return data;
}

GetDataParam::~GetDataParam() {
    if (flag && this->params != nullptr) {
        delete this->params;
    }
    this->params = nullptr;
}
