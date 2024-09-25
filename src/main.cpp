#include"../include/sever.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main(int argc,char* argv[])
{
    // if(argc<3)
    // {
    //     printf("./a.out port respath\n");
    //     exit(0);
    // }
    // int port=atoi(argv[1]);
    //chdir(argv[2]);
    Server s(8989);
    s.init("/home/lorrainewen/CodeLamp/httpserver/res/");
    s.loop();
    return 0;
}