#include <kCGI.h>
#include <fastcgi.h>
#include <kCGI.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>

using namespace kHttpdName;
int kCGI::RequestId = 1;
kCGI::kCGI(string ip,int port){
	int rc;
	int sockfd;
	struct sockaddr_in server_address;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd > 0);

	bzero(&server_address, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(ip.c_str());
	server_address.sin_port = htons(port);

	rc = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
	assert(rc >= 0);

	this->sockfd = sockfd;
	this->requestId = kCGI::RequestId++;
}
kCGI::kCGI(string SockFilePath){
	int rc;
	int sockfd;
	struct sockaddr_un s_un;
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	assert(sockfd > 0);
	s_un.sun_family = AF_UNIX;
	strcpy(s_un.sun_path, SockFilePath.c_str());
	rc = connect(sockfd, (struct sockaddr*)&s_un, sizeof(s_un));
	assert(rc >= 0);

	this->sockfd = sockfd;
	this->requestId = kCGI::RequestId++;
}
kCGI::~kCGI(){
	close(this->sockfd);
}

int kCGI::sendStartRequestRecord(){
	int rc;
	FCGI_BeginRequestRecord beginRecord;

	beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, this->requestId, sizeof(beginRecord.body),0);
	beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

	rc = write(this->sockfd, (char *)&beginRecord, sizeof(beginRecord));
	assert(rc == sizeof(beginRecord));

	return 1;
}
FCGI_Header kCGI::makeHeader(int type, int request,
		int contentLength, int paddingLength){
	FCGI_Header header;
	header.version = FCGI_VERSION_1;
	header.type = (unsigned char)type;
	/* 两个字段保存请求ID */
	header.requestIdB1 = (unsigned char)((requestId >> 8) & 0xff);
	header.requestIdB0 = (unsigned char)(requestId & 0xff);
	/* 两个字段保存内容长度 */
	header.contentLengthB1 = (unsigned char)((contentLength >> 8) & 0xff);
	header.contentLengthB0 = (unsigned char)(contentLength & 0xff);
	/* 填充字节的长度 */
	header.paddingLength = (unsigned char)paddingLength;
	/* 保存字节赋为 0 */
	header.reserved = 0;
	return header;
}
FCGI_BeginRequestBody kCGI::makeBeginRequestBody(int role, int keepConnection){
	FCGI_BeginRequestBody body;
	/* 两个字节保存期望 php-fpm 扮演的角色 */
	body.roleB1 = (unsigned char)((role >> 8) & 0xff);
	body.roleB0 = (unsigned char)(role & 0xff);
	/* 大于0长连接，否则短连接 */
	body.flags = (unsigned char)((keepConnection) ? FCGI_KEEP_CONN : 0);
	bzero(&body.reserved, sizeof(body.reserved));
	return body;
}
