//
// Created by caesar on 2019/9/4.
//

#include <Sqlite.h>
#include <unistd.h>
#include <Log.h>
#include <cstdio>
#include <termios.h>

void show_help() {
    const char *help = "help (http://kekxv.com)\n\n"
                       "-h                  mysql 地址\n"
                       "-P                  mysql 端口\n"
                       "-u                  mysql 账号\n"
                       "-p                  mysql 密码\n"
                       "-d                  mysql 数据库\n"
                       "-q                  mysql 查询语句\n"
                       "-?                  帮助\n"
                       "-v                  日志\n"
                       "\n";
    fprintf(stderr, "%s", help);
}

int main(int argc, char **argv) {

    string databasePath;
    string sql;
    char c;
    while ((c = getopt(argc, argv, "d:q:?v::")) != -1) {
        switch (c) {
            case 'd':
                databasePath = optarg;
                break;
            case 'q':
                sql = optarg;
                break;
            case 'v':
                if (optarg == nullptr) {
                    kHttpdName::Log::setConsoleLevel(3);
                } else {
                    kHttpdName::Log::setConsoleLevel((int) strtol(optarg, nullptr, 10));
                }
                break;
            case '?':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            default:
                printf("1:%s\n",optarg);
                exit(EXIT_SUCCESS);
                break;
        }
    }
    if(databasePath.empty())return -1;
    Sqlite3 sqlite(databasePath);
    if(!sqlite.isConnect()){
        printf("打开失败\n");
        return -2;
    }
    vector<string> headerTitle;
    vector<map<string, string>> data;
    if(sqlite.query(sql,headerTitle,data)>0){

        vector<string> fieldLen;
        vector<size_t> fieldLens;
        size_t sumLen = 0;
        for (const auto &item : headerTitle) {
            size_t len = item.length();
            for (auto da:data) {
                if (len < da[item].length()) {
                    len = da[item].length();
                }
            }
            char buf[100];
            auto L = (len < 2 ? 2 : len) + 2;
            sprintf(buf, "%%%lds  |", L);
            fieldLen.emplace_back(buf);
            fieldLens.push_back(L + 2);
            // printf(buf, item.name.c_str());
            sumLen += L + 2 + 1;
        }

        printf("┏");
        for (size_t l = 0; l < fieldLens.size(); l++) {
            for (size_t i = 0; i < fieldLens[l]; i++) {
                putchar('-');
            }
            if (l < fieldLens.size() - 1) {
                printf("┳");
            }
        }
        // 输出数据
        printf("┓\n");
        printf("|");
        for (size_t i = 0; i < headerTitle.size(); i++) {
            printf(fieldLen[i].c_str(), headerTitle[i].c_str());
        }
        printf("\n┣");
        for (size_t l = 0; l < fieldLens.size(); l++) {
            for (size_t i = 0; i < fieldLens[l]; i++) {
                putchar('-');
            }
            if (l < fieldLens.size() - 1) {
                printf("╋");
            }
        }
        // 输出数据
        printf("┫\n");
        for (auto da:data) {
            printf("|");
            for (size_t i = 0; i < headerTitle.size(); i++) {
                printf(fieldLen[i].c_str(), da[headerTitle[i]].empty() ? "NULL" : da[headerTitle[i]].c_str());
            }
            printf("\n");
        }
        printf("┗");
        for (size_t l = 0; l < fieldLens.size(); l++) {
            for (size_t i = 0; i < fieldLens[l]; i++) {
                putchar('-');
            }
            if (l < fieldLens.size() - 1) {
                printf("┻");
            }
        }
        // 输出数据
        printf("┛\n");
    }
    return 0;
}