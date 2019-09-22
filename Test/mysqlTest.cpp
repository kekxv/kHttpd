//
// 数据库 使用例子
// Created by caesar on 2019/9/3.
//

#include <unistd.h>
#include <MySql.h>
#include <Log.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

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

//参考http://blog.csdn.net/liuchao35758600/article/details/6419499
int getch(void) {
    struct termios tm{}, tm_old{};
    int fd = STDIN_FILENO, c;
    if (tcgetattr(fd, &tm) < 0)
        return -1;
    tm_old = tm;
    cfmakeraw(&tm);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
        return -1;
    c = fgetc(stdin);
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
        return -1;
    if (c == 3) exit(1);  //按Ctrl+C结束退出
    return c;
}

int main(int argc, char *argv[]) {
    string ip = "127.0.0.1";
    unsigned short port = 3306;
    string user = "root";
    string password;
    string database = "mysql";
    string sql;
    //获取参数
    int c;
    while ((c = getopt(argc, argv, "h:P:u:p::d:q:?v::")) != -1) {
        switch (c) {
            case 'h':
                ip = optarg;
                break;
            case 'u':
                user = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'd':
                database = optarg;
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
            case 'P':
                port = strtol(optarg, nullptr, 10);
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
    if (sql.empty()) {
        return 0;
    }
    if (password.empty()) {
        char ch;
        int i = 0;
        char str1[200];
        printf("输入数据库密码: ");
        while ((ch = getch()) != 13 && i < sizeof(str1)) //按回车键退出
        {
            str1[i++] = ch;
            putchar('*');
        }
        putchar('\n');
        str1[i] = '\0';
        password = str1;
    }

    MySql mySql(ip, user, password, database, port);
    MYSQL_RES *res = nullptr;
    unsigned long long ret = mySql.query(&res, sql);
    if (ret > 0) {
        vector<string> fieldLen;
        vector<size_t> fieldLens;
        auto field = MySql::fields(res);
        vector<map<string, string>> data = MySql::GetRows(res);
        size_t sumLen = 0;
        // 输出字段头，并且计算占位符
        // printf("┏┳┓");
        // printf("┣╋┫");
        // printf("┗┻┛");
        for (const auto &item : field) {
            size_t len = item.name.length();
            for (auto da:data) {
                if (len < da[item.name].length()) {
                    len = da[item.name].length();
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
        for (size_t i = 0; i < field.size(); i++) {
            printf(fieldLen[i].c_str(), field[i].name.c_str());
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
            for (size_t i = 0; i < field.size(); i++) {
                printf(fieldLen[i].c_str(), da[field[i].name].empty() ? "NULL" : da[field[i].name].c_str());
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
    MySql::FreeRes(res);

    return 0;
    // map 默认会自动排序
    auto data = mySql.query(sql);
    for (size_t i = 0; i < data.size(); i++) {
        map<string, string> row = data[i];
        if (i == 0) {
            for (const auto &item : row) {
                printf("%s | ", item.first.c_str());
            }
            printf("\n");
        }
        for (const auto &item : row) {
            printf("%s | ", item.second.c_str());
        }
        printf("\n");
    }

    return 0;
}