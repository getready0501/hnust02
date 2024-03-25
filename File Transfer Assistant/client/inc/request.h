#ifndef __REQUEST_H__
#define __REQUEST_H__

/*
    Res_ls:     封装 ls 请求发送给服务器,并接受回复解析结果
    @sockfd:    通信套接字
*/
void Res_ls(int sockfd);

/*
    Res_get:    封装 get 请求发送给服务器,并接受回复解析结果
    @sockfd:    通信套接字
    @filename:  待下载的文件名
    @len:       文件名长度
*/
void Res_get(int sockfd,char *filename,int len);

/*
    Res_put:    封装 put 请求发送给服务器,并等待回复上传文件结果
    @sockfd:    通信套接字
    @filename:  待上传的文件名
*/
void Res_put(int sockfd,char *filename,int len);

/*
    Res_bye:    封装 bye 请求发送给服务器,并等待回复上传文件结果
    @sockfd:    通信套接字
*/
void Res_bye(int sockfd);


#endif