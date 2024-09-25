#ifndef HTTPHANDLER_HEADER
#define HTTPHANDLER_HEADER
#include"../include/tools.h"
#include<arpa/inet.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<errno.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
class HttpHandler
{
    public:
    HttpHandler();
    ~HttpHandler();
    int accepthttpcon(int sfd,int epfd);
    void recvhttprequest(int cfd,int epfd);
    int parsehttprequest(int cfd,const char* requestline);
    void sendheadmsg(int cfd,int status,const char* descr,const char* type,int len);
    void sendfiledata(int cfd,const char* filepath);
    void senddirdata(int cfd,const char* dirpath);
};
#endif