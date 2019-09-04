//
// Created by caesar on 2019/9/3.
//

#include "MySql.h"
#include <my_config.h>
#include <mysql.h>
#include <Log.h>

const char *MySql::TAG = "MySql";

MySql::MySql(const string &IP, const string &User, const string &Password, const string &Database,
             unsigned short PORT) {

    LogI(TAG, "MySQL client version: %s", mysql_get_client_info());
    this->IP = IP;
    this->User = User;
    this->Password = Password;
    this->Database = Database;
    this->PORT = PORT;
    mysql = new MYSQL();
    if (mysql != nullptr) {
        mysql_init(mysql); /* 初始化 */
        int reconnect = 1;
        mysql_options(mysql, mysql_option::MYSQL_OPT_RECONNECT, &reconnect);
        if (mysql_real_connect(mysql, IP.c_str(), User.c_str(), Password.c_str(), Database.c_str(), PORT, nullptr, 0)) {
            LogI(TAG, "数据库连接成功！");
        } else {
            LogE(TAG, "数据库连接失败 : %s", mysql_error(mysql));
        }
    }
}

MySql::~MySql() {
    Free();
    if (mysql != nullptr)
        delete mysql;
}

bool MySql::isConnect() {
    return mysql != nullptr;
}


void MySql::Free() {
    if (mysql != nullptr)
        mysql_close(mysql); /* 关闭连接 */
}

string MySql::GetErrMessage() {
    return mysql_error(mysql);
}

unsigned long long MySql::exec(const string &sql) {
    if (mysql_real_query(mysql, sql.c_str(), sql.length())) {
        LogE(TAG, "%s", mysql_error(mysql));
        return 0;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (!result) {
        auto ret = mysql_num_rows(result);
        mysql_free_result(result);
        return ret;
    } else {
        unsigned int ret = mysql_field_count(mysql);
        if (ret == 0) {
            // 说明执行的是UPDATE/INSERT等这类操作
            auto ret1 = mysql_affected_rows(mysql);
            LogI(TAG, "Query OK, %d rows affected", ret1);
            return ret1;
        } else {
            LogE(TAG, "Error: %s", mysql_error(mysql));
        }
    }
    return 0;
}

unsigned long long MySql::query(MYSQL_RES **result, const string &sql) {
    if (mysql_real_query(mysql, sql.c_str(), sql.length())) {
        LogE(TAG, "%s", mysql_error(mysql));
        return 0;
    }
    *result = mysql_store_result(mysql);
    unsigned long long num_rows = mysql_num_rows(*result);
    return num_rows;
}

void MySql::FreeRes(MYSQL_RES *result) {
    if (result != nullptr)
        mysql_free_result(result);
}

vector<MySql::MysqlFieldInfo> MySql::fields(MYSQL_RES *res) {
    vector<MysqlFieldInfo> fields;
    unsigned int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *_fields;//数组，包含所有field的元数据
    _fields = mysql_fetch_fields(res);
    for (unsigned int i = 0; i < num_fields; ++i) {
        MysqlFieldInfo mysqlFieldInfo;
        if (_fields[i].name)
            mysqlFieldInfo.name = _fields[i].name;
        if (_fields[i].table)
            mysqlFieldInfo.table = _fields[i].table;
        if (_fields[i].db)
            mysqlFieldInfo.db = _fields[i].db;
        if (_fields[i].catalog)
            mysqlFieldInfo.catalog = _fields[i].catalog;
        if (_fields[i].def)
            mysqlFieldInfo.DefaultValue = _fields[i].def;
        mysqlFieldInfo.length = _fields[i].length;
        mysqlFieldInfo.max_length = _fields[i].max_length;
        mysqlFieldInfo.charset = _fields[i].charsetnr;
        mysqlFieldInfo.type = (MysqlFieldTypes) _fields[i].type;
        fields.emplace_back(mysqlFieldInfo);
    }
    return fields;
}

map<string, string> MySql::GetRow(MYSQL_RES *res) {
    map<string, string> rows;
    auto fields = MySql::fields(res);
    //2. 获得每行的数据
    unsigned int num_fields = mysql_num_fields(res);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)return rows;
    unsigned long *lengths;
    lengths = mysql_fetch_lengths(res);
    for (unsigned int i = 0; i < num_fields; ++i) {
//        LogI(TAG, "%s:%s", fields[i].name.c_str(),
//             row[i] ? row[i] : "NULL");
        if (row[i])
            rows[fields[i].name] = row[i];
    }
    return rows;
}

vector<map<string, string>> MySql::GetRows(MYSQL_RES *res) {
    vector<map<string, string>> Datas;
    do {
        auto data = GetRow(res);
        if (data.empty())break;
        Datas.push_back(data);
    } while (true);
    return Datas;
}

vector<map<string, string>> MySql::query(const string &sql) {
    vector<map<string, string>> datas;
    MYSQL_RES *result = nullptr;
    unsigned long long ret = query(&result, sql);
    if (ret == 0) {
        FreeRes(result);
        return datas;
    }
    datas = GetRows(result);
    FreeRes(result);
    return datas;
}

void MySql::beginTransaction() {
    mysql_autocommit(mysql, false);
}

void MySql::commit() {
    mysql_commit(mysql);
    mysql_autocommit(mysql, true);
}

void MySql::rollBack() {
    mysql_rollback(mysql);
    mysql_autocommit(mysql, true);
}


