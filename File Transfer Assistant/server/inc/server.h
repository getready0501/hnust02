#ifndef __SERVER_H__
#define __SERVER_H__


/*
    socket_init:    网络初始化(创建套接字、复用、绑定、监听)
    @ip:            服务器 IP
    @port:          服务器 PORT
    返回值:
        成功返回监听套接字
        失败进程 over
*/
int socket_init(char *ip,char *port);

/*
    socket_end:     扫尾工作
    @socket:        待关闭监听套接字
*/
int socket_end(int sockfd);

//  自定义信号处理函数
void sig_handler(int sig);

/*
    Handler_connect:    处理客户端的请求
    @sockfd:            连接套接字
*/
void Handler_connect(int sockfd);

/*
    Handler: 复用等待 客户端连接,创建新进程进行处理
    @sockfd:        监听套接字
*/
void Handler(int sockfd);

#endif
