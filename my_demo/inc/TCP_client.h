#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include "home.h"
#include "staff.h"

class TCPClient {
private:
    string server_ip;  // 服务器IP地址
    int server_port;        // 服务器端口
    int client_socket;      // 客户端套接字
    mutex socket_mutex; // 用于保护套接字的互斥锁
    atomic<bool> receiving;  // 控制接收线程的标志
    thread receive_thread;  // 接收数据的线程

    void receive_data();  // 接收服务器发送的消息   

public:
    TCPClient(const string& server_ip, int );  // 构造函数
    ~TCPClient();  // 析构函数，关闭连接
    int send_data(const string& data);  // 发送数据
    void start_receiving();  // 启动接收线程
    void stop_receiving();  // 停止接收线程

};

#endif // TCPCLIENT_H
