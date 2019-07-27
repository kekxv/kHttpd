#ifdef WIN32
#pragma once
#endif
#pragma execution_character_set("utf-8")
#include <stdio.h>
#include <vector>
#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#pragma warning(disable:4996)
#else
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#endif

#ifdef LogShowFile
#define LogI(Tag,format,...) kHttpdName::Log::I(Tag,"[" __FILE__ ":%d ] " format, __LINE__,##__VA_ARGS__)
#define LogD(Tag,format,...) kHttpdName::Log::D(Tag,"[" __FILE__ ":%d ] " format, __LINE__,##__VA_ARGS__)
#define LogE(Tag,format,...) kHttpdName::Log::E(Tag,"[" __FILE__ ":%d ] " format, __LINE__,##__VA_ARGS__)
#else
#define LogI(Tag,format,...) kHttpdName::Log::I(Tag,format, ##__VA_ARGS__)
#define LogD(Tag,format,...) kHttpdName::Log::D(Tag,format, ##__VA_ARGS__)
#define LogE(Tag,format,...) kHttpdName::Log::E(Tag,format, ##__VA_ARGS__)
#endif
//#define LOG(format, ...) fprintf(stdout, format, __VA_ARGS__)

#ifndef kHttpd_Log
#define kHttpd_Log
using namespace std;
namespace kHttpdName {
	class Log
	{
	public:

		static long long GetTime();
		/*
		输出一般信息
		*/
		static void I(const char *TAG, const char* Message,...);
		/*
		输出调试信息
		*/
		static void D(const char *TAG, const char* Message,...);
		/*
		输出调试二进制字符串
		*/
		static void D_HX(const char *TAG,int len, unsigned char* Message,const char* Tip = "");
		/*
		输出错误信息
		*/
		static void E(const char *TAG, const char* Message,...);

		/*
		设置日志保存程度
		*/
		static int setLevel(int level);
		/*
		设置日志输出程度
		*/
		static int setConsoleLevel(int level);

		/*
		 * 添加输出黑名单
		 * 不输出 指定 TAG 标签信息
		 */
		static int AddBlacklist(const char *TAG);
		/*
		 * 删除一项黑名单
		 */
		static int RmBlacklist(const char *TAG);

		/*
		 * 添加输出白名单
		 */
		static int AddWhitelist(const char *TAG);
		/*
		 * 删除一项白名单
		 */
		static int RmWhitelist(const char *TAG);
		/*
		 * 开启白名单模式
		 */
		static bool EnWhitelist;

		/*
		 * 字节流转换为十六进制字符串
		 */
		static void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);

		/*
		获取动态库路径
		*/
		static unsigned long GetDllPath(char* curDir, int size);

		/*
		日志保存程度
		*/
		static int  level;
		/*
		命令行输出级别
		*/
		static int  ConsoleLevel;

	private:
		/*
		 * 白名单列表
		 */
		static vector<const char*> Whitelist;
		/*
		 * 黑名单列表
		 */
		static vector<const char*> Blacklist;
		/*
		输出信息
		*/
		static void Write(
#ifdef WIN32
		SYSTEMTIME
#else
		struct tm*
#endif
  Time, const char * Type, const char *TAG, const char* Message);

		/*
		日志保存
		*/
		static void SaveLog(
#ifdef WIN32
		SYSTEMTIME
#else
		struct tm*
#endif
  Time, const char * Type, const  char *TAG, const char* Message, const char * LogFilePath = NULL);

		/*
		设置为白色黑底
		*/
		static void setConsoleTextAttribute();

		/*
		获取当前时间
		*/
#ifdef WIN32
		static SYSTEMTIME getNowTime();
#else
		static struct tm* getNowTime();
#endif

	};
}

#endif
