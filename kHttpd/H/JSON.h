//
// Created by caesar on 2019-08-12.
//

#ifndef KHTTP_JSON_H
#define KHTTP_JSON_H

#include <string>
using namespace std;

struct cJSON;

namespace kHttpdName {
    class JSON {
    public:
        static JSON parse(const string& json);
        explicit JSON(const string& json);
        explicit JSON(const char* json = "{}");
        explicit JSON(int num);
        explicit JSON(double num);
        explicit JSON(bool flag);
        ~JSON();
        string toString() const;

        JSON operator[] (const string& key);
        JSON operator[] (size_t index);

        JSON& operator=(const JSON &json );
        JSON& operator=(double data);
        JSON& operator=(int data);
        JSON& operator=(const string& data);
        JSON& operator=(const char* data);
        JSON& operator=(bool data);

        string GetString();
        int GetNumber();
        double GetDouble();
        bool GetBoolean();

        int GetType();

        void Add(const JSON& json,const string& key);
        void Add(const JSON& json);
        void Add(const string& key,const JSON& json);
        void Add(const string& key,const string &value);
        void Remove(int index);
        void Remove(const string& key);

    private:
        explicit JSON(cJSON* cJson);
        cJSON *cJson = nullptr;
        cJSON * GetNewCJson() const;
        void Free();

    };
}


#endif //KHTTP_JSON_H
