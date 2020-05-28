#pragma once
#include "../common/consts.h"
#include "../common/heads.h"
#include "../hall_server/process_log.h"
#include "base_data.h"

class serverBase {
protected:
    /*
     * @brief 初始化socket
     * @param port 监听的端口
     */
    serverBase(int);
    virtual ~serverBase();
    void Work();
    /*
     * @brief 发送数据给单个客户端, 数据是序列化出的字节流
     * @param clientFd 客户端fd
     * @param length 要发送的数据长度
     */
    void SendDataToClient(int clientFd, int length);
    /*
     * @brief 处理具体逻辑，由子类实现
     */
    virtual void HandleEvent(int,int) {}
    /**
     * @brief 处理连接上的ip和端口信息,由子类实现
     */
    virtual void HandleNetIp(char* ,int, int) {}
    /*
     * @brief 处理关闭连接时的逻辑，由子类实现
     */
    virtual void HandleClose(int) {}
    baseData bData;

private:
    int port, server_fd;
    struct epoll_event* events;
    struct epoll_event ev;
    int epoll_fd;
    struct sockaddr_in server_addr;
    void Init();
    void InitEpoll();
};

