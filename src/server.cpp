#include"../include/sever.h"
#include"../include/httphandler.h"
Server::Server(int port):port(port)
{
    
}
Server::~Server()
{

}
void Server::init(const char* respath)
{
    int res=chdir(respath);
    if(res==-1)
    {
        perror("chdir error");
    }
    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1)
    {
        perror("socket error");
    }
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    saddr.sin_addr.s_addr=INADDR_ANY;
    int opt=1;
    res=setsockopt(sfd,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));
    if(res==-1)
    {
        perror("setsockopt error");
    }
    res=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res==-1)
    {
        perror("bind error");
    }
    res=listen(sfd,100);
    if(res==-1)
    {
        perror("listen error");
    }
    epfd=epoll_create(1);
    if(epfd==-1)
    {
        perror("epoll_create error");
    }
    struct epoll_event event;
    event.data.fd=sfd;
    event.events=EPOLLIN|EPOLLET;
    res=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
    if(res==-1)
    {
        perror("epoll_ctl error");
    }
}
void Server::loop()
{
    int res;
    HttpHandler httphandler;
    while(true)
    {
        int num=epoll_wait(epfd,revent,1024,-1);
        if(num==-1)
        {
            perror("epoll_wait error");
            continue;
        }
        for(int i=0;i<num;i++)
        {
            int curfd=revent[i].data.fd;
            if(curfd==sfd)
            {
                res=httphandler.accepthttpcon(sfd,epfd);
                if(res==-1)
                {
                    break;
                }
            }
            else
            {
                httphandler.recvhttprequest(curfd,epfd);
            }
        }
    }
}