#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <functional>

#include "../common/consts.h"
#include "base_data.h"

class socketBase {
protected:
    socketBase(int);
    virtual ~socketBase();
    void Work();
    virtual void HandleEvent(int) {}
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

