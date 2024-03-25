#ifndef __HANDLER_H__
#define __HANDLER_H__

/*
    Handler_connect:    处理客户端的请求
    @con_fd:            连接套接字
*/
void Handler_connect(int con_fd);

/*
    Handle_ls:      处理客户端 ls 的请求
    @sockfd:        通信套接字
*/
void Handle_ls(int sockfd);

/*
    Handle_ls:      处理客户端 get 的请求
    @sockfd:        通信套接字
    @filename:      待获取的文件名
*/
void Handle_get(int con_fd, char *filename);

/*
    Handle_ls:      处理客户端 get 的请求
    @sockfd:        通信套接字
    @filename:      待上传的文件名
    @len:           上传的文件名长度
*/
void Handle_put(int con_fd, char *filename, int size);

/*
    Handle_ls:      处理客户端 get 的请求
    @sockfd:        通信套接字
    @filename:      待上传的文件名
    @len:           上传的文件名长度
*/
void Handle_bye(int con_fd);

#endif