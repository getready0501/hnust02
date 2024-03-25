#ifndef __CLIENT_H__
#define __CLIENT_H__

/*
    socket_init:    网络初始化(创建套接字)
    返回值:
        成功返回通信套接字
        失败进程 over
*/
int socket_init();

/*
    socket_end:     扫尾工作
    @socket:        待关闭监听套接字
*/
int socket_end(int sockfd);

/*
    connect_server: 连接服务器准备通信
    @sock_fd:   通信套接字
    @ip:            服务器 IP
    @port:          服务器 PORT
    返回值:
        成功返回监听套接字
        失败进程 over
*/
void connect_server(int socket,char *ip,char *port);

/*
    Handler:    对客户端的请求进行对应的响应
    @sock_fd:   通信套接字
*/
void Handler(int sock_fd);

#endif