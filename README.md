# kHttpd 服务端软件

## v0.4

- 增加 MySQL 支持，请查看 mysqlTest.cpp 例子

## v0.3

- 增加车牌识别代码
```shell script
kHttpdDemo -v -C ../libCarNumOcr/model -w ../web_car/
```

## v0.2

### 支持功能：

1. 开启日志输出
2. 支持 `php cgi` 模式 (不支持 `POST`)

### 后期支持：

1. JSON 接口
2. 文档完善
3. 支持 `PHP CGI` `POST` 模式

## v0.1

### 支持功能：

1. 支持自定义路由
2. 支持静态文件输出
3. 支持输出数据流
4. 可通过 (*GET)[key],(*POST)[key],(*COOKIE)[key] 读取对应值

### 后期支持：

1. JSON 接口
2. 文档完善