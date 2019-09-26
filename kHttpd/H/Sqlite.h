//
// Created by caesar on 2019/9/4.
//

#ifdef BUILD_SQLITE3
#ifndef KHTTP_SQLITE3_H
#define KHTTP_SQLITE3_H

#include <string>
#include <map>
#include <vector>
using namespace std;
typedef struct sqlite3 sqlite3;
class Sqlite3 {
public:
    static const char *TAG;

    explicit Sqlite3(const string& databasePath);
    ~Sqlite3();
    bool isConnect();
    void Free();

    string GetErrMessage();
    void beginTransaction();
    void commit();
    void rollBack();

    unsigned long long exec(const string &sql);

    unsigned long long query(const string &sql,vector<string> &headerTitle,vector<map<string, string>> &data);

private:
    sqlite3 *db = nullptr;
    string DatabasePath;
    bool isConnected = false;
};


#endif //KHTTP_SQLITE3_H
#endif //BUILD_SQLITE3
