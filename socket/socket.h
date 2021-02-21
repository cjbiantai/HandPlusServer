#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/epoll.h>
#include <stdio.h>

#include "../common/consts.h"
#include "base_data.h"

class socketBase {
public:
    socketBase(int);
    virtual ~socketBase();
    virtual void Work(){}

protected:
    int port, server_fd;
    struct epoll_event* events;
    struct epoll_event ev;
    int epoll_fd;
    baseData bData;
private:
    struct sockaddr_in server_addr;
    void Init();
    void InitEpoll();
};

