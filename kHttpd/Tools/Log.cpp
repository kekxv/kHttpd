#include "Log.h"
#ifndef WIN32
#include <iconv.h>
#endif
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif
#ifdef ANDROID_SO

#include <android/log.h>

//#define  LOG_TAG    "AndroidHDevice"
//#define  LOGI(LOG_TAG,...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define  LOGE(LOG_TAG,...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif
namespace kHttpdName {

/*
	 * 白名单列表
	 */
vector<const char *> Log::Whitelist;
/*
	 * 黑名单列表
	 */
vector<const char *> Log::Blacklist;
/*
	 * 开启白名单模式
	 */
bool Log::EnWhitelist = false;

/*
	日志保存程度
	*/
int Log::level = 0;
/*
	命令行输出级别
	*/
int Log::ConsoleLevel = 0;

/*
	输出一般信息
	*/
void Log::I(const char *TAG, const char *Message, ...)
{
	char *_Message;
	_Message = (char *)malloc(2048 * sizeof(char));
	if (_Message == NULL)return;
	memset(_Message, 0, 2048 * sizeof(char));
	va_list args;
	va_start(args, Message);

#ifdef WIN32
	vsprintf(_Message, Message, args);
#else
	vsnprintf(_Message, 2048 * sizeof(char), Message, args);
#endif
	va_end(args);
#ifdef WIN32
	SYSTEMTIME
#else
	struct tm *
#endif
	Time = Log::getNowTime();
	if (Log::ConsoleLevel >= 3)
	{
#ifdef WIN32
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
#else
		printf("\033[1;32m");
#endif
		Log::Write(Time, (char *)"I", TAG, _Message);
		Log::setConsoleTextAttribute();
	}
	if (Log::level >= 3)
	{
		Log::SaveLog(Time, (char *)"I", TAG, _Message);
	}
	free(_Message);
}
long long Log::GetTime()
{
#ifdef _WIN32
// 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    long long tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) / 10 / 1000;
    return tt;
#else
    timeval tv;
    gettimeofday(&tv, 0);
    return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
#endif // _WIN32
    return 0;
}

/*
	输出调试信息
	*/
void Log::D(const char *TAG, const char *Message, ...)
{
	char *_Message;
	_Message = (char *)malloc(2048 * sizeof(char));
	if (_Message == NULL)return;
	memset(_Message, 0, 2048 * sizeof(char));
	va_list args;
	va_start(args, Message);

#ifdef WIN32
	vsprintf(_Message, Message, args);
#else
	vsnprintf(_Message, 2048 * sizeof(char), Message, args);
#endif
	va_end(args);
#ifdef WIN32
	SYSTEMTIME
#else
	struct tm *
#endif
	Time = Log::getNowTime();
	if (Log::ConsoleLevel >= 2)
	{
#ifdef WIN32
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
#else
		printf("\033[1;33m");
#endif
		Log::Write(Time, (char *)"D", TAG, _Message);
		Log::setConsoleTextAttribute();
	}
	if (Log::level >= 2)
	{
		Log::SaveLog(Time, (char *)"D", TAG, _Message);
	}
	free(_Message);
}
/*
	输出调试二进制字符串
	*/
void Log::D_HX(const char *TAG, int len, unsigned char *Message, const char *Tip)
{
	if (!(Log::ConsoleLevel > 2 || Log::level > 2))return;
	char *responsData;
	responsData = (char *)malloc((len * 2 + 1) * sizeof(char));
	if (responsData == NULL)return;
	memset(responsData, 0, (len * 2 + 1) * sizeof(char));
	ByteToHexStr(Message, responsData, len);
	D(TAG, "[%s] [ %s ]", Tip, responsData);
	free(responsData);
}

/*
	输出错误信息
	*/
void Log::E(const char *TAG, const char *Message, ...)
{
	char *_Message;
	_Message = (char *)malloc(2048 * sizeof(char));
	if (_Message == NULL)return;
	memset(_Message, 0, 2048 * sizeof(char));
	va_list args;
	va_start(args, Message);

#ifdef WIN32
	vsprintf(_Message, Message, args);
#else
	vsnprintf(_Message, 2048 * sizeof(char), Message, args);
#endif
	va_end(args);
#ifdef WIN32
	SYSTEMTIME
#else
	struct tm *
#endif
	Time = Log::getNowTime();
	if (Log::ConsoleLevel >= 1)
	{
#ifdef WIN32
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
#else
		printf("\033[1;31m");
#endif
		Log::Write(Time, (char *)"E", TAG, _Message);
		Log::setConsoleTextAttribute();
	}
	if (Log::level >= 1)
	{
		Log::SaveLog(Time, (char *)"E", TAG, _Message);
	}
	free(_Message);
}

/*
	输出信息
	*/
void Log::Write(
#ifdef WIN32
	SYSTEMTIME
#else
	struct tm *
#endif
		Time,
	const char *Type, const char *TAG, const char *Message)
{
	if (EnWhitelist)
	{
		//不在白名单不输出
		if (Whitelist.empty() || Whitelist.end() == std::find_if(Whitelist.begin(), Whitelist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); }))
		{
			return;
		}
	}
	else
	{
		//黑名单不输出
		if (!Blacklist.empty() && Blacklist.end() > std::find_if(Blacklist.begin(), Blacklist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); }))
		{
			return;
		}
	}
#ifdef WIN32
	::printf("[%4d/%02d/%02d %02d:%02d:%02d][%s] %s : %s \n",
			 Time.wYear,
			 Time.wMonth,
			 Time.wDay,
			 Time.wHour,
			 Time.wMinute,
			 Time.wSecond,
			 Type,
			 TAG,
			 Message);
#else
#if ANDROID_SO
        __android_log_print((strstr(Type,"E") != NULL) ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG, TAG,
                            "\n*\n/*************************\n    [%s] %s\n*************************/\n\n",Type,Message);
#else
	::printf("[%4d/%02d/%02d %02d:%02d:%02d][%s] %s : %s \n",
			 (1900 + Time->tm_year),
			 (1 + Time->tm_mon),
			 Time->tm_mday,
			 Time->tm_hour,
			 Time->tm_min,
			 Time->tm_sec,
			 Type,
			 TAG,
			 Message);
#endif
#endif
}

/*
	设置日志保存程度
	*/
int Log::setLevel(int level)
{
#ifdef LogShow
	if (level > 3)
		Log::level = 3;
#else
	if (level > 3)
		Log::level = 3;
#endif
	else if (level < 0)
		Log::level = 0;
	else
		Log::level = level;
	return Log::level;
}
/*
	命令行输出级别
	*/
int Log::setConsoleLevel(int level)
{
#ifdef LogShow
	if (level > 3)
		Log::ConsoleLevel = 3;
#else
	if (level > 3)
		Log::ConsoleLevel = 3;
#endif
	else if (level < 0)
		Log::ConsoleLevel = 0;
	else
		Log::ConsoleLevel = level;
	return Log::ConsoleLevel;
}

/*
	日志保存
	*/
void Log::SaveLog(
#ifdef WIN32
	SYSTEMTIME
#else
	struct tm *
#endif
		Time,
	const char *Type, const char *TAG, const char *Message, const char *LogFilePath)
{
	char *sz = NULL;
	if (LogFilePath == NULL)
	{
		sz = (char *)malloc(256 * sizeof(char));
		memset(sz, 0x00, 256 * sizeof(char));
		if (strlen(sz) == 0)
		{
			GetDllPath(sz, 256);
#ifdef WIN32
			sprintf(sz, "%s\\log", sz);
#else
			snprintf(sz, 256 * sizeof(char), "log");
#endif
		}
		LogFilePath = sz;
	};
#ifdef WIN32
	// 目录不存在创建目录
	if (_access(LogFilePath, 0) != 0)
		if (_mkdir(LogFilePath) != 0){
			if(sz!=NULL)free(sz);
			return;
		}
#else
	if (access(LogFilePath, 0) != 0)
		if (mkdir(LogFilePath, 0755) != 0){
			if(sz!=NULL)free(sz);
			return;
		}
#endif

	char *F_PATH = NULL;
	F_PATH = (char *)malloc(256 * sizeof(char));
	if(F_PATH == NULL){
		if(sz!=NULL)free(sz);
		return;
	}
	memset(F_PATH, 0x00, 256 * sizeof(char));

#ifdef WIN32
	sprintf(F_PATH, "%s\\%4d-%02d-%02d %02d.log", LogFilePath,
			Time.wYear,
			Time.wMonth,
			Time.wDay,
			Time.wHour);
#else
	snprintf(F_PATH, 256 * sizeof(char), "%s/%4d-%02d-%02d %02d.log", LogFilePath,
			 (1900 + Time->tm_year),
			 (1 + Time->tm_mon),
			 Time->tm_mday,
			 Time->tm_hour);
#endif

	FILE *fp = NULL; //需要注意

#ifdef WIN32
	if ((fp = fopen(F_PATH, "a+")) == NULL){
		free(F_PATH);
		free(sz);
		return;
	}
	fprintf(fp, "[%4d/%02d/%02d %02d:%02d:%02d][%s] %s : %s\n",
			Time.wYear,
			Time.wMonth,
			Time.wDay,
			Time.wHour,
			Time.wMinute,
			Time.wSecond,
			Type,
			TAG,
			Message); //从控制台中读入并在文本输出
#else
	if ((fp = fopen(F_PATH, "a+")) == NULL){
		if(F_PATH!=NULL)free(F_PATH);
		if(sz!=NULL)free(sz);
		return;
	}
	fprintf(fp, "[%4d/%02d/%02d %02d:%02d:%02d][%s] %s : %s\n",
			(1900 + Time->tm_year),
			(1 + Time->tm_mon),
			Time->tm_mday,
			Time->tm_hour,
			Time->tm_min,
			Time->tm_sec,
			Type,
			TAG,
			Message); //从控制台中读入并在文本输出
#endif
	fclose(fp);
	fp = NULL; //需要指向空，否则会指向原打开文件地址
	if(F_PATH!=NULL)free(F_PATH);
	if(sz!=NULL)free(sz);
	return;
}
/*
	设置为白色黑底
	*/
void Log::setConsoleTextAttribute()
{
#ifdef WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
							FOREGROUND_RED |	   // 前景色_红色
								FOREGROUND_GREEN | // 前景色_绿色
								FOREGROUND_BLUE);  // 前景色_蓝色
#else
	printf("\e[0m");
#endif
}

	/*
	获取当前时间
	*/
#ifdef WIN32
SYSTEMTIME Log::getNowTime()
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	//printf("%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d/n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sys.wDayOfWeek);
	return sys;
}
#else
struct tm *Log::getNowTime()
{
	time_t timep;
	time(&timep);
	return localtime(&timep);
}
#endif
/*
	 * 字节流转换为十六进制字符串
	 */
void Log::ByteToHexStr(const unsigned char *source, char *dest, int sourceLen)
{
	if (sourceLen <= 0)
		return;
	int i;
	unsigned char highByte, lowByte;
	for (i = 0; i < sourceLen; i++)
	{
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;
		highByte += 0x30;
		if (highByte > 0x39)
			dest[i * 2] = highByte + 0x07;
		else
			dest[i * 2] = highByte;
		lowByte += 0x30;
		if (lowByte > 0x39)
			dest[i * 2 + 1] = lowByte + 0x07;
		else
			dest[i * 2 + 1] = lowByte;
	}
	return;
}

/*
	 * 添加一个白名单
	 */
int Log::AddWhitelist(const char *TAG)
{
	if (!Whitelist.empty() && Whitelist.end() > std::find_if(Whitelist.begin(), Whitelist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); }))
	{
		return 0;
	}
	Whitelist.push_back(TAG);
	return 1;
}
/*
	 * 删除一个白名单
	 */
int Log::RmWhitelist(const char *TAG)
{
	if (!Whitelist.empty())
	{
		vector<const char *>::iterator it;
		it = std::find_if(Whitelist.begin(), Whitelist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); });
		if (it < Whitelist.end())
		{
			Whitelist.erase(it);
			return 1;
		}
	}
	return 0;
}
/*
	 * 添加一个黑名单
	 */
int Log::AddBlacklist(const char *TAG)
{
	if (!Blacklist.empty() && Blacklist.end() > std::find_if(Blacklist.begin(), Blacklist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); }))
	{
		return 0;
	}
	Blacklist.push_back(TAG);
	return 1;
}
/*
	 * 删除一个黑名单
	 */
int Log::RmBlacklist(const char *TAG)
{
	if (!Blacklist.empty())
	{
		vector<const char *>::iterator it;
		it = std::find_if(Blacklist.begin(), Blacklist.end(), [TAG](const char *obj) { return (strcmp(TAG, obj) == 0); });
		if (it < Blacklist.end())
		{
			Blacklist.erase(it);
			return 1;
		}
	}
	return 0;
}

/*
	获取动态库路径
	*/
unsigned long Log::GetDllPath(char *curDir, int size)
{
	char *fn, *p;
	fn = (char *)malloc(256 * sizeof(char));
	unsigned long ret;
	memset(curDir, 0x00, size);
#ifdef WIN32
	MEMORY_BASIC_INFORMATION mbi;
	memset(fn, 0x00, 256 * sizeof(char));
	ret = GetModuleFileName(((::VirtualQuery(Log::GetDllPath, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL), fn, size);
	p = strrchr(fn, '\\');
	strncpy(curDir, fn, p - fn);
	//上句函数第2实参这样写以防止文件在当前目录下时因p=NULL而出错
#endif
	free(fn);
	return ret;
}
}
