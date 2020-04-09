#pragma once
#include "../common/consts.h"
#include "../common/heads.h"
#include "base_data.h"

class serverBase {
protected:
    serverBase(int);
    virtual ~serverBase();
    void Work();
    virtual void HandleEvent(int,int) {}
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

