#include <stdio.h>
#include <stdlib.h>             //  atoi 头文件
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>         //  网络地址相关的头文件
#include <arpa/inet.h>          //  转换头文件
#include <unistd.h>             //  read/write/close 头文件
#include "client.h"
#include "request.h"


/*
    socket_init:    网络初始化(创建套接字)
    返回值:
        成功返回通信套接字
        失败进程 over
*/
int socket_init()
{
    //  创建一个套接字
    int sockfd = socket(AF_INET,SOCK_STREAM,0);     //  IPv4 流式套接字 使用不知名的私有应用层协议
    if(sockfd == -1)
    {
        perror("server socket error");
        exit(-1);
    }
    return sockfd;
}

/*
    socket_end:     扫尾工作
    @socket:        待关闭监听套接字
*/
int socket_end(int sockfd)
{
    close(sockfd);
}

/*
    connect_server: 连接服务器准备通信
    @sock_fd:   通信套接字
    @ip:            服务器 IP
    @port:          服务器 PORT
    返回值:
        成功返回监听套接字
        失败进程 over
*/
void connect_server(int sockfd,char *ip,char *port)
{
    //  2、准备一个 server 网络地址
	//	2.1 IPv4 网络结构体保存 server 网络地址
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));		        //	先把所有的成员变量值设置为 0
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));          //  指定 PORT 
	addr.sin_addr.s_addr = inet_addr(ip);       //  指定 IP
	

    //  3、连接 server
    if(connect(sockfd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
    {
        perror("server connect error");
        exit(-4);
    }
}


/*
    Handler:    对客户端读取终端输入/获取触摸事件,根据不同的情况,执行对应请求,接受回复
    @sockfd:    通信套接字
*/
void Handler(int sockfd)
{
    //  1、获取输入
    char buf[30] = {0};
    //	读文件描述符集合
    fd_set rfds;
    while(1)
    {
        //  1、对多路复用的参数初始化		
        //  1.1 先清空文件描述符
        FD_ZERO(&rfds);
        //	1.2 将 标准输入 加入到 rfds 文件描述符集合
        FD_SET(STDIN_FILENO,&rfds);	
        //  1.3 超时时间为 2s 50us
        struct timeval timeout = {2,50};	

        //  2、多路复用
        int ret = select(STDIN_FILENO + 1,&rfds,NULL,NULL,&timeout);
		if(ret == -1)		//	出错
		{
			perror("select error");
            break;
		}
        else if(ret == 0)   //  超时
        {
            //printf("input timeout\n");
            continue;
        }
        if(FD_ISSET(STDIN_FILENO,&rfds))	    //	标准输入 是否在 就绪的文件描述符集合中
        {
            //  1、获取输入
            char buf[128] = {0};
            fgets(buf,128,stdin);    //  可以获取空白字符

            //  2、根据不同的情况,执行对应封装请求并发送给服务器,接受回复
            if(strncmp(buf,"ls",2) == 0)        //  ls 带参自行处理
            {
                Res_ls(sockfd);
            }
            else if(strncmp(buf,"get",3) == 0) 
            {
                //  先获取要下载的文件名
                unsigned char filename[128] = {0};
                int i = 3,j = 0;
                while(buf[i] == ' ')        //  跳过 空格影响
                {
                    i++;
                }
                while(buf[i] != '\n')       //  依次获取文件
                {
                    filename[j++] = buf[i++];
                }
                Res_get(sockfd,filename,j);
            }
            else if(strncmp(buf,"put",3) == 0) 
            {
                //  保存获取要上传的文件名
                unsigned char filename[128] = {0};
                int i = 3,j = 0;
                while(buf[i] == ' ')        //  跳过 空格影响
                {
                    i++;
                }
                while(buf[i] != '\n')       //  依次获取文件
                {
                    filename[j++] = buf[i++];
                }
                Res_put(sockfd,filename,j);
            }
            else if(strncmp(buf,"bye",3) == 0)        //  ls 带参自行处理
            {
                Res_bye(sockfd);
            }
        }
    }
}