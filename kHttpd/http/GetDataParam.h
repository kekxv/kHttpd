//
// Created by caesar on 2019-07-26.
//

#ifndef KHTTP_GETDATAPARAM_H
#define KHTTP_GETDATAPARAM_H

#include <string>

struct evhttp_request;
struct evkeyvalq;

using namespace std;
namespace kHttpdName {
    inline static string &trimEnd(string &text, const string& t = " \n\r\t") {
        if (!text.empty()) {
            text.erase(text.find_last_not_of(t) + 1);
        }
        return text;
    }

    inline static string &trimStart(string &text, const string& t = " \n\r\t") {
        if (!text.empty()) {
            text.erase(0, text.find_first_not_of(t));
        }
        return text;
    }

    inline static string &trim(string &text, const string& t = " \n\r\t") {
        text = trimStart(text, t);
        text = trimEnd(text, t);
        return text;
    }

    class GetDataParam {
    public:
        explicit GetDataParam(struct ::evhttp_request *req);

        explicit GetDataParam(const string &keyValueString);

        explicit GetDataParam(struct ::evkeyvalq *params);

        ~GetDataParam();

        string operator[](const string &key);    //重载"[]"操作符
    private:
        struct ::evkeyvalq *params = nullptr;
        bool flag = true;

        static string DecodeURI(const string &url);
    };

}


#endif //KHTTP_GETDATAPARAM_H
