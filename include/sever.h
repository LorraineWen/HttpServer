#ifndef SERVER_HEADER
#define SERVER_HEADER
#include<arpa/inet.h>
#include<sys/epoll.h>
class Server
{
    public:
    Server(int port=8989);
    ~Server();
    void init(const char* respath);
    void loop();
    private:
    int sfd;
    int port;
    struct sockaddr_in saddr;
    int epfd;
    struct epoll_event revent[1024];
};
#endif