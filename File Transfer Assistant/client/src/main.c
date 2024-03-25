#include <stdio.h>
#include "client.h"

/*
    argv[0]     可执行文件
    argv[1]     server IP
    argv[2]     server PORT
*/
int main(int argc,char *argv[])
{
    if(argc < 3)
    {
        printf("please input:program_file server_ip server_port\n");
        return -1;
    }
    //  1、网络初始化(创建套接字)
    int sockfd = socket_init();

    //  2、连接服务器,需要服务器网络地址
    connect_server(sockfd,argv[1],argv[2]);

    //  3、读取终端输入/获取触摸事件
    Handler(sockfd);        //  简单应用层协议
    
    //  4、扫尾  
    socket_end(sockfd);
    return 0;
}

