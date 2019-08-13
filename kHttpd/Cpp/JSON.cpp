//
// Created by caesar on 2019-08-12.
//

#include "JSON.h"
#include <cJSON.h>

using namespace kHttpdName;

JSON JSON::parse(const string &json) {
    return JSON(json);
}

JSON::JSON(const string &json) {
    cJson = cJSON_Parse(json.c_str());
    if (cJson == nullptr)cJson = cJSON_CreateString(json.c_str());
}

JSON::JSON(const char *json) {
    cJson = cJSON_Parse(json);
    if (cJson == nullptr)cJson = cJSON_CreateString(json);
}


JSON::JSON(cJSON *cJson) {
    this->Free();
    this->cJson = cJson;
    string json = this->toString();
    this->cJson = cJSON_Parse(json.c_str());
}

JSON::~JSON() {
    this->Free();
}

string JSON::toString() const {
    if (cJson == nullptr)return "";
    char *buf = nullptr;
    buf = cJSON_PrintUnformatted(cJson);
    string _ = buf;
    free(buf);//释放资源
    return _;
}


string JSON::GetString() {
    if (cJson == nullptr) {
        return "";
    }
    if(cJson->valuestring != nullptr)
        return cJson->valuestring;
    if(cJson->string != nullptr)
        return cJson->string;
    return "";
}

int JSON::GetNumber() {
    if (cJson == nullptr) {
        return 0;
    }
    return cJson->valueint;
}

bool JSON::GetBoolean() {
    if (cJson == nullptr) {
        return false;
    }
    return cJson->type & cJSON_True;
}

double JSON::GetDouble() {
    if (cJson == nullptr) {
        return 0.0;
    }
    return cJson->valuedouble;
}

void JSON::Free() {
    if (cJson != nullptr) {
        if (!(cJson->type & cJSON_IsReference))
            cJSON_Delete(cJson);
        cJson->type = cJSON_IsReference;
        cJson = nullptr;
    }
}

void JSON::Add(const JSON &json, const string &key) {
    if (key.empty()) {
        if (json.cJson->type & cJSON_Array) {
            cJSON_AddItemToArray(cJson, json.GetNewCJson());
        }
    } else {
        cJSON_AddItemToObject(cJson, key.c_str(), json.GetNewCJson());
    }
}

void JSON::Add(const string &key, const JSON &json) {
    Add(json, key);
}

void JSON::Add(const JSON &json) {
    cJSON_AddItemToArray(cJson, json.GetNewCJson());
}

int JSON::GetType() {
    if (cJson == nullptr)return -1;
    return cJson->type;
}

void JSON::Remove(int index) {
    cJSON_DeleteItemFromArray(cJson, index);
}

void JSON::Remove(const string &key) {
    cJSON_DeleteItemFromObject(cJson, key.c_str());
}

JSON::JSON(double num) {
    *this = num;
}

JSON::JSON(int num) {
    *this = num;
}

JSON::JSON(bool flag) {
    *this = flag;
}

cJSON *JSON::GetNewCJson() const {
    string jsonStr = this->toString();
    return cJSON_Parse(jsonStr.c_str());
}

JSON &JSON::operator=(const double data) {
    this->Free();
    this->cJson = cJSON_CreateNumber(data);
    return *this;
}

JSON &JSON::operator=(const int data) {
    *this = (double) data;
    return *this;
}

JSON &JSON::operator=(const char *data) {
    *this = string(data);
    return *this;
}

JSON &JSON::operator=(const string &data) {
    this->Free();
    if (data.empty()) {
        this->cJson = cJSON_CreateNull();
    } else
        this->cJson = cJSON_CreateString(data.c_str());
    return *this;
}

JSON &JSON::operator=(bool data) {
    this->Free();
    this->cJson = data ? cJSON_CreateTrue() : cJSON_CreateFalse();
    return *this;
}

JSON &JSON::operator=(const JSON &json) {
    this->Free();
    this->cJson = json.GetNewCJson();
    return *this;
}

JSON JSON::operator[](const string &key) {
    cJSON *itemName = cJSON_GetObjectItem(cJson, key.c_str());
    JSON json(itemName);
//    cJSON_Delete(itemName);
    return json;
}

JSON JSON::operator[](size_t index) {
    cJSON *itemName = cJSON_GetArrayItem(cJson, index);
    JSON json(itemName);
//    cJSON_Delete(itemName);
    return json;
}




