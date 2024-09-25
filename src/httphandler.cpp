#include"../include/httphandler.h"
HttpHandler::HttpHandler()
{

}
HttpHandler::~HttpHandler()
{

}
int HttpHandler::accepthttpcon(int sfd,int epfd)
{
    struct sockaddr_in caddr;
    int caddrlen=sizeof(caddr);
    int cfd=accept(sfd,(struct sockaddr*)&caddr,(socklen_t*)&caddrlen);
    if(cfd==-1)
    {
        perror("accept error");
        return -1;
    }
    int flag=fcntl(cfd,F_GETFL);
    flag|=O_NONBLOCK;
    fcntl(cfd,F_SETFL,flag);
    struct epoll_event ev;
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=cfd;
    int res=epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
    if(res==-1)
    {
        perror("epoll_ctl error");
        return -1;
    }
    char cip[50]={'\0'};
    inet_ntop(AF_INET,&caddr.sin_addr.s_addr,cip,sizeof(cip));
    printf("连接到了%s:%d\n",cip,ntohs(caddr.sin_port));
    return 0;
}
void HttpHandler::recvhttprequest(int cfd,int epfd)
{
    char buffer[4096]={'\0'};//将收到的数据存入这里
    char tmp[1024]={'\0'};//接受1k数据
    int total=0;
    while(true)
    {
        int len=recv(cfd,tmp,sizeof(tmp),0);
        if(len==-1)
        {
            if(errno==EAGAIN)//说明http的请求数据已经读取完毕了
            {
               //将get请求的请求行里面的数据拿出来
               char* end=strstr(buffer,"\r\n");//end指向/r
               int datalen=end-buffer;
               buffer[datalen]='\0';//字符串截断
               parsehttprequest(cfd,buffer);
            }
            else
            {
                perror("recv error");         
            }
            break;
        }
        else if(len==0)
        {
            printf("连接断开\n");
            epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,NULL);
            close(cfd);
            break;
        }
        else
        {
            if(total+len<sizeof(buffer))
            {
                memcpy(buffer+total,tmp,len);
                total+=len;
            }
        }
    }
}
int HttpHandler::parsehttprequest(int cfd,const char* requestline)
{
    //GET /hello/text.a http/1.1
    //将请求行的三部分进行拆分，获取提交方式，和请求的文件路径，剩下的不要了
    char method[10]={'\0'};
    char path[1024]={'\0'};
    sscanf(requestline,"%[^ ] %[^ ]",method,path);
    //判断请求方式是否是get
    if(strcmp(method,"GET"))
    {
        return -1;
    }
    //得到资源目录
    char* filepath=NULL;
    Tools::get_instance()->decode(path,path);
    if(strcasecmp(path,"/")==0)
    {
        filepath="./";
    }
    else
    {
        filepath=path+1;//得到home/lorrainewen/CodeLamp/httpserver/res/
    }
    //判断用户访问的是文件还是目录(stat函数)
    struct stat filestat;
    int res=stat(filepath,&filestat);
    if(res==-1)
    {
        //发送404页面
        sendheadmsg(cfd,404,"Not Found",Tools::get_instance()->getfiletype(".html"),-1);
        sendfiledata(cfd,"404.html");
    }
    else if(S_ISDIR(filestat.st_mode))//是目录，要遍历目录，将目录里面的文件名发送给客户端
    {
        sendheadmsg(cfd,200,"OK",Tools::get_instance()->getfiletype(".html"),-1);
        senddirdata(cfd,filepath);
    }
    else//是文件，就发送文件内容
    {
        sendheadmsg(cfd,200,"OK",Tools::get_instance()->getfiletype(filepath),filestat.st_size);
        sendfiledata(cfd,filepath);
    } 
    return 0;
}
//由于http是基于tcp协议的，所以http响应的头部内容(请求行，请求头，空行)和数据内容可以分开发送
//分开发送头部和数据内容使得服务器可以在生成响应的过程中逐步发送数据，
//而不需要等到整个响应内容都准备好后才发送。这就是流式传输，
//它允许客户端可以逐步接收数据而不需要等待整个响应的内容全部就绪。
void HttpHandler::sendheadmsg(int cfd,int status,const char* descr,const char* type,int len)
{
    char headbuffer[4096]={'\0'};
    sprintf(headbuffer,"http/1.1 %d %s\r\n",status,descr);
    sprintf(headbuffer+strlen(headbuffer),"Content-Type: %s\r\n",type);
    sprintf(headbuffer+strlen(headbuffer),"Content-Length:%d\r\n\r\n",len);
    int res=send(cfd,headbuffer,strlen(headbuffer),0);
    if(res==-1)perror("send headmsg error");
}
void HttpHandler::sendfiledata(int cfd,const char* filepath)
{
    char buffer[1024]={'\0'};
    int len;
    int fd=open(filepath,O_RDONLY);
    if(fd==-1)
    {
        perror("open file error");
    }
    while(true)
    {
        int len=read(fd,buffer,sizeof(buffer));
        if(len==-1)
        {
            perror("read filedata error");
            break;
        }
        else if(len==0)
        {
            break;
        }
        else
        {
            int res=send(cfd,buffer,len,0);
            if(res==-1)
            {
                perror("send filedata error");
            }
            usleep(100);//为了避免发送太快，浏览器上一次的没解析完，这次就发过去了
            //这样解析过程中就会丢失一部分数据，导致图片只加载一半
        }
    }
    close(fd);
}
void HttpHandler::senddirdata(int cfd,const char* dirpath)
{
    struct dirent **namelist;
    int num=scandir(dirpath,&namelist,NULL,alphasort);
    if(num==-1)
    {
        perror("scandir error");
    }
    char buffer[4096]={'\0'};
    sprintf(buffer,"<html><head><title>%s</title></head><body><table>",dirpath);
    for(int i=0;i<num;i++)
    {
        char* name=namelist[i]->d_name;
        char subpath[1024];
        sprintf(subpath,"%s/%s",dirpath,name);
        struct stat filestat;
        int ret=stat(subpath,&filestat);
        if(ret==-1)
        {
            perror("stat error");
            continue;
        }
        if(S_ISDIR(filestat.st_mode))
        {   
            //如果是目录，超链接跳转的时候必须在%s后面加上/
            sprintf(buffer+strlen(buffer),"<tr><td><a href=\"%s/\">%s</a></td><td>%d</td></tr>",name,name,filestat.st_size);
        }
        else
        {
            sprintf(buffer+strlen(buffer),"<tr><td><a href=\"%s\">%s</a></td><td>%d</td></tr>",name,name,filestat.st_size);
        }
        //边生成边发送，避免buffer变满,tcp可以保证数据连续
        ret=send(cfd,buffer,strlen(buffer),0);
        memset(buffer,0,sizeof(buffer));
        free(namelist[i]);//释放用过的namelist
    }
    sprintf(buffer,"</table></body></html>");
    num=send(cfd,buffer,strlen(buffer),0);
    if(num==-1)
    {
        perror("send error");
    }
    free(namelist);
}