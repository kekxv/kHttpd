//
// Created by caesar on 2019/9/4.
//

#ifdef BUILD_SQLITE3

#include <sqlite3.h>
#include "Sqlite.h"
#include <Log.h>

const char *Sqlite3::TAG = "Sqlite3";

Sqlite3::Sqlite3(const string &databasePath) {
    DatabasePath = databasePath;
    int ret = sqlite3_open(databasePath.c_str(), &db);
    isConnected = (ret == SQLITE_OK);
    LogI(TAG, "数据库【 %s 】开启：%s", databasePath.c_str(), isConnect() ? "成功" : GetErrMessage().c_str());
}

Sqlite3::~Sqlite3() {
    Free();
}

bool Sqlite3::isConnect() {
    return isConnected;
}

void Sqlite3::Free() {
    LogI(TAG, "关闭数据库");
    if (db != nullptr)
        sqlite3_close(db);
    db = nullptr;
    isConnected = false;
}

string Sqlite3::GetErrMessage() {
    return sqlite3_errmsg(db);
}

unsigned long long Sqlite3::exec(const string &sql) {
    int ret = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        LogE(TAG, "操作失败:%s", GetErrMessage().c_str());
        return 0;
    }
    return sqlite3_changes(db);
}


unsigned long long  Sqlite3::query(const string &sql,vector<string> &headerTitle,vector<map<string, string>> &data) {
    char **dbResult = nullptr;
    char *errmsg = nullptr;
    int nRow = 0;
    int nColumn = 0;
    int ret = sqlite3_get_table(db, sql.c_str(), &dbResult, &nRow, &nColumn, &errmsg);
    if (SQLITE_OK == ret) {
        for (int j = 0; j < nColumn; j++) {
            headerTitle.emplace_back(dbResult[j]);
        }
        //查询成功
        int index = nColumn; //前面说过 dbResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据
        for (int i = 0; i < nRow; i++) {
            map<string, string> item;
            for (int j = 0; j < nColumn; j++) {
                item[dbResult[j]] = dbResult[index];
                ++index;
            }
            data.push_back(item);
        }
        LogI(TAG, "操作成功");
    } else {
        LogE(TAG, "操作失败:%s", GetErrMessage().c_str());
    }
    sqlite3_free_table(dbResult);
    return data.size();
}
void Sqlite3::beginTransaction() {
    exec("BEGIN TRANSACTION;");
}

void Sqlite3::commit() {
    exec("END TRANSACTION;");
}

void Sqlite3::rollBack() {
    exec("ROLLBACK;");
}


#endif //BUILD_SQLITE3
