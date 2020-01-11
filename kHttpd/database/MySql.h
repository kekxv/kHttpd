//
// Created by caesar on 2019/9/3.
//

#ifdef BUILD_MYSQL
#ifndef KHTTP_MYSQL_H
#define KHTTP_MYSQL_H

#include <vector>
#include <map>
#include <string>

struct st_mysql;
typedef st_mysql MYSQL;
struct st_mysql_res;
typedef st_mysql_res MYSQL_RES;
using namespace std;


class MySql {
public:

    typedef enum {
        MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
        MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
        MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
        MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP,
        MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
        MYSQL_TYPE_DATE, MYSQL_TYPE_TIME,
        MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
        MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
        MYSQL_TYPE_BIT,
        MYSQL_TYPE_TIMESTAMP2,
        MYSQL_TYPE_DATETIME2,
        MYSQL_TYPE_TIME2,
        MYSQL_TYPE_JSON = 245,
        MYSQL_TYPE_NEWDECIMAL = 246,
        MYSQL_TYPE_ENUM = 247,
        MYSQL_TYPE_SET = 248,
        MYSQL_TYPE_TINY_BLOB = 249,
        MYSQL_TYPE_MEDIUM_BLOB = 250,
        MYSQL_TYPE_LONG_BLOB = 251,
        MYSQL_TYPE_BLOB = 252,
        MYSQL_TYPE_VAR_STRING = 253,
        MYSQL_TYPE_STRING = 254,
        MYSQL_TYPE_GEOMETRY = 255
    } MysqlFieldTypes;

    typedef struct {
        string name;
        string table;
        string db;
        string catalog;
        string DefaultValue;
        unsigned long length;
        unsigned long max_length;
        unsigned int charset;
        MysqlFieldTypes type;
    } MysqlFieldInfo;
    static const char *TAG;

    explicit MySql(const string &IP = "127.0.0.1", const string &User = "root", const string &Password = "",
                   const string &Database = "mysql",
                   unsigned short PORT = 3306);

    ~MySql();

    bool isConnect();

    void Free();

    string GetErrMessage();

    void beginTransaction();
    void commit();
    void rollBack();


    unsigned long long exec(const string &sql);

    unsigned long long query(MYSQL_RES **result, const string &sql);
    vector<map<string,string>> query(const string &sql);
    static void FreeRes(MYSQL_RES*result);

    static vector<MysqlFieldInfo> fields(MYSQL_RES *res);

    static map<string,string> GetRow(MYSQL_RES *res);
    static vector<map<string,string>> GetRows(MYSQL_RES *res);

private:
    MYSQL *mysql = nullptr;
    string IP = "127.0.0.1";
    unsigned short PORT = 3306;
    string User = "root";
    string Password = "";
    string Database = "mysql";
};


#endif //KHTTP_MYSQL_H
#endif //BUILD_MYSQL
