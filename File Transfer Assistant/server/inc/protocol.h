#ifndef __PROCOTOL_H__
#define __PROCOTOL_H__

//  指定服务器的目录路径
#define FTP_BOOT "/home/linuxos/ftpboot"

//  指定客户端的目录路径
#define CLITP_BOOT "/home/linuxos/ftpclient"

//  命令与应答数据一般采用整数值命令号形式，可以通过定义枚举类型
enum CMD_NO
{
    FTP_CMD_LS = 1024,      //  获取文件列表的命令号
    FTP_CMD_GET,            //  下载文件的命令号
    FTP_CMD_PUT,            //  上传文件的命令号
    FTP_CMD_BYE,
    //  ......
};
/*
enum ERR_NO
{
    //  ......
};
*/
//  应用层协议的 "数据格式"
/* 
1、客户端发送请求数据
    请求是否带参问题!!! 可能需要可能不需要
    数据格式:
        头标(包头) pkg_len cmd_no arg1_len arg1_data arg2_len arg2_len ... 尾标(包尾)
    解析:
        0xC0        //  1byte 数据包的第一个字节,每个数据包都是以 0xC0 开头;实际应用中,为了保证包头唯一性,会设置多个字节为包头
        pkg_len     //  4bytes 该数据包的长度,小端存储(先存放低字节)
        cmd_no      //  4bytes 该数据包的命令号,小端存储(先存放低字节)
        arg1_len    //  4bytes 该数据包的命令的第一个参数的长度,小端存储(先存放低字节)
        arg1_data   //  arg1_len bytes 该数据包的命令的第一个参数的内容,
        arg2_len    //  4bytes 该数据包的命令的第二个参数的长度,小端存储(先存放低字节)
        ...
        0xC0        //  1byte 数据包的最后一个字节,每个数据包都是以 0xC0 结尾;实际应用中,为了保证包尾唯一性,会设置多个字节为包尾
    eg:
        CMD:ls
        0xC0 pkg_len cmd_no 0xC0
        1       4       4    1
        unsigned char request[10] = {0};
        int pkg_len = 10;
        int cmd_no = FTP_CMD_LS;
        int i = 0;

        //  依次按照格式封装数据
        //  包头   
        request[i++] = 0xC0;
        //  数据包长度   
        request[i++] = pkg_len & 0xff;          //  小端模式保存低字节
        request[i++] = (pkg_len >> 8) & 0xff;   //  
        request[i++] = (pkg_len >> 16) & 0xff;  //  
        request[i++] = (pkg_len >> 24) & 0xff;  //  小端模式保存高字节
        //  数据包命令   
        request[i++] = cmd_no & 0xff;           //  小端模式保存低字节
        request[i++] = (cmd_no >> 8) & 0xff;    //  
        request[i++] = (cmd_no >> 16) & 0xff;   //  
        request[i++] = (cmd_no >> 24) & 0xff;   //  小端模式保存高字节
        //  包尾   
        request[i++] = 0xC0;

        实际数据中有 0xC0,解决方式:
            发送时,将数据内容中 
                    0xC0 --> 0xdd 0xdb
                    0xdd --> 0xdd 0xdc
            接收时,将数据内容中 
                    0xdd 0xdb --> 0xC0
                    0xdd 0xdc --> 0xdd 
    eg again:
        CMD:get filename
        0xC0 pkg_len cmd_no arg_len arg_data 0xC0
        1       4       4      4    arg_len   1
        int arg_len = strlen(filename);
        unsigned char request[14+arg_len] = {0};
        int pkg_len = 14+arg_len;
        int cmd_no = FTP_CMD_GET;
        int i = 0;

        //  依次按照格式封装数据
        //  包头   
        request[i++] = 0xC0;
        //  数据包长度   
        request[i++] = pkg_len & 0xff;          //  小端模式保存低字节
        request[i++] = (pkg_len >> 8) & 0xff;   //  
        request[i++] = (pkg_len >> 16) & 0xff;  //  
        request[i++] = (pkg_len >> 24) & 0xff;  //  小端模式保存高字节
        //  数据包命令   
        request[i++] = cmd_no & 0xff;           //  小端模式保存低字节
        request[i++] = (cmd_no >> 8) & 0xff;    //  
        request[i++] = (cmd_no >> 16) & 0xff;   //  
        request[i++] = (cmd_no >> 24) & 0xff;   //  小端模式保存高字节
        //  参数长度   
        request[i++] = arg_len & 0xff;           //  小端模式保存低字节
        request[i++] = (arg_len >> 8) & 0xff;    //  
        request[i++] = (arg_len >> 16) & 0xff;   //  
        request[i++] = (arg_len >> 24) & 0xff;   //  小端模式保存高字节
        //  参数内容
        strncpy(request+i,filename,arg_len);
        i += arg_len;
        //  包尾   
        request[i++] = 0xC0; 
 
2、服务器回复客户端数据
    数据格式:
        头标(包头) pkg_len cmd_no reply_len reply_result reply_data 尾标(包尾)
    解析:
        0xC0            //  1byte 数据包的第一个字节,每个数据包都是以 0xC0 开头;实际应用中,为了保证包头唯一性,会设置多个字节为包头
        pkg_len         //  4bytes 该数据包的长度,小端存储(先存放低字节)
        cmd_no          //  4bytes 该数据包的命令号,小端存储(先存放低字节)
        reply_len       //  4bytes 该数据包的回复内容的长度(reply_result+reply_data),小端存储(先存放低字节)
        reply_result    //  1bytes 服务器对客户端请求处理结果 1代表成功,0代表失败,小端存储(先存放低字节)
        reply_data      //  服务器对客户端请求处理内容 
                            失败:   
                                4bytes 代表错误码,由程序员设定,小端存储(先存放低字节)
                            成功:
                                ls  所有文件的名字,各个文件名之间使用某个方式隔开,长度未知
                                get 文件大小,下一次开始再循环发送文件内容
                                ...
        0xC0        //  1byte 数据包的最后一个字节,每个数据包都是以 0xC0 结尾;实际应用中,为了保证包尾唯一性,会设置多个字节为包尾

    eg again:
        ls 的回复
        0xC0 pkg_len cmd_no reply_len reply_result reply_data 0xC0
          1     4       4       4          1       reply_len-1  1

        all_filename 所有的文件名信息,通过文件 IO 操作,读取目录下的所有文件名

        int reply_len = strlen(all_filename) + 1;
        int pkg_len = 14+reply_len;
        unsigned char reply[pkg_len] = {0};
        int cmd_no = FTP_CMD_LS;
        int i = 0;

        //  依次按照格式封装数据
        //  包头   
        reply[i++] = 0xC0;
        //  数据包长度   
        reply[i++] = pkg_len & 0xff;          //  小端模式保存低字节
        reply[i++] = (pkg_len >> 8) & 0xff;   //  
        reply[i++] = (pkg_len >> 16) & 0xff;  //  
        reply[i++] = (pkg_len >> 24) & 0xff;  //  小端模式保存高字节
        //  数据包命令   
        reply[i++] = cmd_no & 0xff;           //  小端模式保存低字节
        reply[i++] = (cmd_no >> 8) & 0xff;    //  
        reply[i++] = (cmd_no >> 16) & 0xff;   //  
        reply[i++] = (cmd_no >> 24) & 0xff;   //  小端模式保存高字节
        //  数据包回复长度   
        reply[i++] = reply_len & 0xff;           //  小端模式保存低字节
        reply[i++] = (reply_len >> 8) & 0xff;    //  
        reply[i++] = (reply_len >> 16) & 0xff;   //  
        reply[i++] = (reply_len >> 24) & 0xff;   //  小端模式保存高字节、
        //  处理情况 成功/失败   
        reply[i++] = 0x01;
        //  处理结果内容
        strncpy(reply+i,all_filename,reply_len-1);
        i += reply_len - 1;
        //  包尾   
        reply[i++] = 0xC0;
    ---------------------------------------------
    get 的回复
        成功:
            0xC0 pkg_len cmd_no reply_len reply_result filesize 0xC0
              1     4       4       4          1           4      1
            再循环发送文件内容,以文件大小作为标识
        失败:
            0xC0 pkg_len cmd_no reply_len reply_result ERR_NO 0xC0
              1     4       4       4          1         4      1 
*/
#endif