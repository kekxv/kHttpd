//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTPD_KCGI_H
#define KHTTPD_KCGI_H

#include <string>
#include <stdio.h>
#include <fastcgi.h>
using namespace std;
namespace kHttpdName{
	class kCGI{
	public:
		kCGI(string ip,int port);
		kCGI(string SockFilePath);
		~kCGI();
		int sendStartRequestRecord();

	private:

		//生成头部
		FCGI_Header makeHeader(int type, int request, 
		                    int contentLength, int paddingLength);
		//生成发起请求的请求体
		FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection);
	private:
		int sockfd = 0;
		int requestId = 0;
		static int RequestId;

	};
}


#endif //KHTTPD_KCGI_H
