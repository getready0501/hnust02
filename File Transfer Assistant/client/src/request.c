#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> /* See NOTES */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> //  read/write/close 头文件
#include "request.h"
#include "protocol.h"
/*
    Res_ls:     封装 ls 请求发送给服务器,并接受回复解析结果
    @sockfd:    通信套接字
*/
void Res_ls(int sockfd)
{
    //  1、封装 ls 请求
    /*
        CMD:ls
          0xC0 pkg_len cmd_no 0xC0
            1       4     4    1
    */
    unsigned char request[10] = {0};
    int pkg_len = 10;
    int cmd_no = FTP_CMD_LS;
    int i = 0;

    //  依次按照格式封装数据
    //  包头
    request[i++] = 0xC0;
    //  数据包长度
    request[i++] = pkg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (pkg_len >> 8) & 0xff;  //
    request[i++] = (pkg_len >> 16) & 0xff; //
    request[i++] = (pkg_len >> 24) & 0xff; //  小端模式保存高字节
    //  数据包命令
    request[i++] = cmd_no & 0xff;         //  小端模式保存低字节
    request[i++] = (cmd_no >> 8) & 0xff;  //
    request[i++] = (cmd_no >> 16) & 0xff; //
    request[i++] = (cmd_no >> 24) & 0xff; //  小端模式保存高字节
    //  包尾
    request[i++] = 0xC0;

    //  2、发送封装好的 ls 数据包给 服务器
    write(sockfd, request, i);

    /*
        ls 的回复
        0xC0 pkg_len cmd_no reply_len reply_result reply_data 0xC0
          1     4       4       4          1            4      1
    */
    //  3、接受服务器的数据回复
    //  3.1 先找到数据包的第一个字节 包头 0xC0
    unsigned char ch;
    do
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    } while (ch != 0xc0); //  循环结束, ch == 0xc0
    //  防止是上一个数据包的包尾
    while (ch == 0xc0)
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    }
    //  循环结束,ch != 0xc0 是数据包的第二个字节
    //  也就是说 ch 保存了 pkg_len 的第一个字节(低字节)

    //  3.2 直接全部读取,可能存在一个 BUG,数据内容有 0xc0
    unsigned char reply[128] = {0}; //  保存服务器发来的数据包,没有包头和包尾的
    i = 0;
    while (ch != 0xc0)
    {
        reply[i++] = ch;
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 服务器关闭时出现
        {
            break;
        }
    }
    //  i 就是数据包的有效内容(没有包头和包尾)的数量 = pkg_len - 2
    //  3.3.1 验证数据长度是否出现问题
    pkg_len = reply[0] | (reply[1] << 8) | (reply[2] << 16) | (reply[3] << 24);
    if (pkg_len - 2 != i) //   理论和实际获取的数据数量不匹配
    {
        printf("message len error,pkg_len = %d,i = %d\n", pkg_len, i);
        //  可以丢弃该次操作,并回复给 客户端当前操作有一个错误
        //  也可以结束
        return;
        //  忽略不管,continue
    }
    //  3.3.2 验证数据命令是否出现问题
    cmd_no = reply[4] | (reply[5] << 8) | (reply[6] << 16) | (reply[7] << 24);
    if (cmd_no != FTP_CMD_LS) //   理论和实际获取的数据命令不匹配
    {
        printf("message CMD_NO error,cmd_no = %d,FTP_CMD_LS = %d\n", cmd_no, FTP_CMD_LS);
        return;
    }
    //  3.3.3 验证服务器操作的结果
    if (reply[12] != 1)
    {
        printf("server operation error\n");
        return;
    }
    //  3.4 经过一些列验证结果无误

    // 拿出文件个数
    int filenum = reply[13] | (reply[14] << 8) | (reply[15] << 16) | (reply[16] << 24);
    printf("=====filenum = %d=====\n", filenum);

    while (filenum--)
    {
        unsigned char buf[1024] = {0};
        read(sockfd, buf, 256);
        if (buf[255] == 1)
        {
            printf("\033[1;32;40m%s\033[0m\n", buf);
            continue;
        }
        printf("\033[1;34;40m%s\033[0m\n", buf);
    }
}

/*
    Res_get:    封装 get 请求发送给服务器,并接受回复解析结果
    @sockfd:    通信套接字
    @filename:  待下载的文件名
    @len:       文件名 长度
*/
void Res_get(int sockfd, char *filename, int len)
{
    //  1、封装 get 请求,并发送给服务器
    /*
        CMD:get filename
        0xC0 pkg_len cmd_no arg_len arg_data 0xC0
        1       4       4      4    arg_len   1
    */
    int arg_len = len;
    unsigned char request[14 + len];
    int pkg_len = 14 + len;
    int cmd_no = FTP_CMD_GET;
    int i = 0;

    //  依次按照格式封装数据
    //  包头
    request[i++] = 0xC0;
    //  数据包长度
    request[i++] = pkg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (pkg_len >> 8) & 0xff;  //
    request[i++] = (pkg_len >> 16) & 0xff; //
    request[i++] = (pkg_len >> 24) & 0xff; //  小端模式保存高字节
    //  数据包命令
    request[i++] = cmd_no & 0xff;         //  小端模式保存低字节
    request[i++] = (cmd_no >> 8) & 0xff;  //
    request[i++] = (cmd_no >> 16) & 0xff; //
    request[i++] = (cmd_no >> 24) & 0xff; //  小端模式保存高字节
    //  参数长度
    request[i++] = arg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (arg_len >> 8) & 0xff;  //
    request[i++] = (arg_len >> 16) & 0xff; //
    request[i++] = (arg_len >> 24) & 0xff; //  小端模式保存高字节
    //  参数内容
    strncpy(request + i, filename, arg_len);
    i += arg_len;
    //  包尾
    request[i++] = 0xC0;
    //  发送封装好的 ls 数据包给 服务器
    write(sockfd, request, i);

    //  2、接受服务器的回复,处理成功/失败
    /*
        成功:
            0xC0 pkg_len cmd_no reply_len reply_result filesize 0xC0
              1     4       4       4          1           4      1
            再循环发送文件内容,以文件大小作为标识
        失败:
            0xC0 pkg_len cmd_no reply_len reply_result ERR_NO 0xC0
              1     4       4       4          1        4       1
    */
    //  3.1 先找到数据包的第一个字节 包头 0xC0
    unsigned char ch;
    do
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    } while (ch != 0xc0); //  循环结束, ch == 0xc0
    //  防止是上一个数据包的包尾
    while (ch == 0xc0)
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    }
    //  循环结束,ch != 0xc0 是数据包的第二个字节
    //  也就是说 ch 保存了 pkg_len 的第一个字节(低字节)

    //  3.2 直接全部读取,可能存在一个 BUG,数据内容有 0xc0
    unsigned char reply[20] = {0}; //  保存服务器发来的数据包,没有包头和包尾的
    i = 0;
    while (ch != 0xc0)
    {
        reply[i++] = ch;
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 服务器关闭时出现
        {
            break;
        }
    }
    //  i 就是数据包的有效内容(没有包头和包尾)的数量 = pkg_len - 2
    //  3.3.1 验证数据长度是否出现问题
    pkg_len = reply[0] | (reply[1] << 8) | (reply[2] << 16) | (reply[3] << 24);
    if (pkg_len - 2 != i) //   理论和实际获取的数据数量不匹配
    {
        printf("message len error,pkg_len = %d,i = %d\n", pkg_len, i);
        //  可以丢弃该次操作,并回复给 客户端当前操作有一个错误
        //  也可以结束
        return;
        //  忽略不管,continue
    }
    //  3.3.2 验证数据命令是否出现问题
    cmd_no = reply[4] | (reply[5] << 8) | (reply[6] << 16) | (reply[7] << 24);
    if (cmd_no != FTP_CMD_GET) //   理论和实际获取的数据命令不匹配
    {
        printf("message CMD_NO error,cmd_no = %d,FTP_CMD_GET = %d\n", cmd_no, FTP_CMD_GET);
        return;
    }
    //  3.3.3 验证服务器操作的结果
    if (reply[12] != 1)
    {
        printf("server operation error\n");
        return;
    }
    //  3.3.4 服务器操作成功,读取文件大小
    int size = reply[13] | (reply[14] << 8) | (reply[15] << 16) | (reply[16] << 24);
    if (size == 0x0111)
    {
        printf("get file error,file not exit\n");
        printf("check the file,or try again\n");
        return;
    }

    //  3、处理成功就能一直接受文件内容
    //  3.1 本地创建一个文件
    char pathname[128] = {0};
    sprintf(pathname, "%s/%s", CLITP_BOOT, filename);
    int fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    //  3.2 获取文件内容写入
    int recv_buf = 0; //  保存接受的数量
    unsigned char buf[1024];

    double rate = 0.05; // 每下载5%的数据就刷新进度条
    int index = 1;
    char str[21];
    memset(str, '\0', sizeof(str));
    char tmp[5] = {'-', '\\', '|', '/', '\0'};

    while (recv_buf < size)
    {
        int r = read(sockfd, buf, 100);
        if (r > 0)
        {
            recv_buf += write(fd, buf, r);
        }
        if (((double)recv_buf / (double)size) >= rate)
        {
            str[index - 1] = '#';
            if (index <= 14)
                printf("\033[1;31;40m%s[%d%%%c]\033[0m\r", str, index * 5, tmp[index % 4]);
            else if (index <= 14)
                printf("\033[1;34;40m%s[%d%%%c]\033[0m\r", str, index * 5, tmp[index % 4]);
            else
                printf("\033[1;32;40m%s[%d%%%c]\033[0m\r", str, index * 5, tmp[index % 4]);
            fflush(stdout);
            usleep(1000);
            index++;
            rate = 0.05 * index;
        }
    }
    printf("\n");
    //  3.3 关闭文件
    close(fd);
    printf("get over\n");
}

/*
    Res_put:    封装 put 请求发送给服务器,并接受回复解析结果
    @sockfd:    通信套接字
    @filename:  待上传的文件名
    @len:       文件名 长度
*/
void Res_put(int sockfd, char *filename, int len)
{
    //  1、先判断文件是否存在,存在获取其大小
    char pathname[128] = {0};
    sprintf(pathname, "%s/%s", CLITP_BOOT, filename);
    struct stat statbuf;
    if (stat(pathname, &statbuf) == -1)
    {
        perror("file is not exist");
    } //  不存在 返回值为 -1

    int arg_len = len;
    unsigned char request[18 + len];
    int pkg_len = 18 + len;
    int cmd_no = FTP_CMD_PUT;
    int i = 0;

    //  2、封装 get 请求,并发送给服务器
    /*
        CMD:get filename
        0xC0 pkg_len cmd_no arg_len filesize arg_data 0xC0
        1       4       4      4       4     arg_len   1
    */
    //  依次按照格式封装数据
    //  包头
    request[i++] = 0xC0;
    //  数据包长度
    request[i++] = pkg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (pkg_len >> 8) & 0xff;  //
    request[i++] = (pkg_len >> 16) & 0xff; //
    request[i++] = (pkg_len >> 24) & 0xff; //  小端模式保存高字节
    //  数据包命令
    request[i++] = cmd_no & 0xff;         //  小端模式保存低字节
    request[i++] = (cmd_no >> 8) & 0xff;  //
    request[i++] = (cmd_no >> 16) & 0xff; //
    request[i++] = (cmd_no >> 24) & 0xff; //  小端模式保存高字节
    //  参数长度
    request[i++] = arg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (arg_len >> 8) & 0xff;  //
    request[i++] = (arg_len >> 16) & 0xff; //
    request[i++] = (arg_len >> 24) & 0xff; //  小端模式保存高字节
    //  数据包 filesize
    request[i++] = statbuf.st_size & 0xff;         //  小端模式保存低字节
    request[i++] = (statbuf.st_size >> 8) & 0xff;  //
    request[i++] = (statbuf.st_size >> 16) & 0xff; //
    request[i++] = (statbuf.st_size >> 24) & 0xff; //  小端模式保存高字节
    //  参数内容
    strncpy(request + i, filename, arg_len);
    i += arg_len;

    //  包尾
    request[i++] = 0xC0;
    //  发送封装好的 put 数据包给 服务器
    if (write(sockfd, request, i) == -1)
    {
        perror("write error");
    }

    //  3、循环发送文件内容(文件存在)
    // 提示正在进行数据上传
    printf("upload doing !!!\n");
    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        // 提示文件打开失败，需要检查，并且返回上一步，再接收带参命令
        perror("open pathname error,check the pathname");
        return;
        //  处理一下
    }

    //  循环发送文件内容给服务器
    unsigned char buf[1024] = {0};
    while (1)
    {
        int r = read(fd, buf, 1024); //  从本地读取
        if (r > 0)
        {
            write(sockfd, buf, r); //  发送给服务器
        }
        else if (r == 0)
        {
            break;
        }
        else
        {
            perror("read pathname error,check the pathname");
        }
    }
    close(fd);

    printf("put over\n");
}

/*
    Res_bye:    封装 bye 请求发送给服务器,并接受回复结果,并且关闭客户端
    @sockfd:    通信套接字
*/
void Res_bye(int sockfd)
{
    //  1、封装 bye 请求
    /*
        CMD:bye
          0xC0 pkg_len cmd_no 0xC0
            1       4     4    1
    */
    unsigned char request[10] = {0};
    int pkg_len = 10;
    int cmd_no = FTP_CMD_BYE;
    int i = 0;

    //  依次按照格式封装数据
    //  包头
    request[i++] = 0xC0;
    //  数据包长度
    request[i++] = pkg_len & 0xff;         //  小端模式保存低字节
    request[i++] = (pkg_len >> 8) & 0xff;  //
    request[i++] = (pkg_len >> 16) & 0xff; //
    request[i++] = (pkg_len >> 24) & 0xff; //  小端模式保存高字节
    //  数据包命令
    request[i++] = cmd_no & 0xff;         //  小端模式保存低字节
    request[i++] = (cmd_no >> 8) & 0xff;  //
    request[i++] = (cmd_no >> 16) & 0xff; //
    request[i++] = (cmd_no >> 24) & 0xff; //  小端模式保存高字节
    //  包尾
    request[i++] = 0xC0;

    //  2、发送封装好的 bye 数据包给 服务器
    write(sockfd, request, i);

    /*
        bye 的回复
          0xC0 pkg_len cmd_no 0xC0
            1       4     4    1
    */
    //  3、接受服务器的数据回复
    //  3.1 先找到数据包的第一个字节 包头 0xC0
    unsigned char ch;
    do
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    } while (ch != 0xc0); //  循环结束, ch == 0xc0
    //  防止是上一个数据包的包尾
    while (ch == 0xc0)
    {
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 对方关闭时出现
        {
            break;
        }
    }
    //  循环结束,ch != 0xc0 是数据包的第二个字节
    //  也就是说 ch 保存了 pkg_len 的第一个字节(低字节)

    //  3.2 直接全部读取,可能存在一个 BUG,数据内容有 0xc0
    unsigned char reply[128] = {0}; //  保存服务器发来的数据包,没有包头和包尾的
    i = 0;
    while (ch != 0xc0)
    {
        reply[i++] = ch;
        if (read(sockfd, &ch, 1) <= 0) //  <0 出错  =0 服务器关闭时出现
        {
            break;
        }
    }

    //  3.3.2 验证数据命令是否出现问题
    cmd_no = reply[4] | (reply[5] << 8) | (reply[6] << 16) | (reply[7] << 24);
    if (cmd_no == FTP_CMD_BYE) //   理论和实际获取的数据命令匹配
    {
        printf("connect over\n");
        close(sockfd);
        exit(-1);
    }
}