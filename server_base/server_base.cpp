#include "server_base.h"

serverBase::serverBase(int port) {
    this -> port = port;
    Init();
    InitEpoll();
}

serverBase::~serverBase() {
    free(events);
}

void serverBase::Init() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) {
        printf("create socket error, errno = %d, (%s)\n", errno, strerror(errno));
        exit(-1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int value = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&value, sizeof(value));

    int bind_ret = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(bind_ret == -1) {
         printf("bind error: errno = %d, (%s)\n", errno, strerror(errno));
         exit(-1);
    }else {
        printf("bind [0.0.0.0:%d] ok!\n", port);
    }

    int listen_ret = listen(server_fd, BACKLOG);
    if(listen_ret == -1) {
        printf("listen error:errno = %d, (%s)\n", errno, strerror(errno));
        exit(-1);
    }else {
        printf("start listening on socket fd [%d] ... \n", server_fd);
    }
}

void serverBase::InitEpoll() {
    epoll_fd = epoll_create(1);
    ev.data.fd = server_fd;
    ev.events = EPOLLIN;

    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    if(ret == -1) {
        printf("epoll_ctl error: errno = %d, (%s)\n", errno, strerror(errno));
        exit(-1);
    }
    events = (struct epoll_event*)malloc(sizeof(ev)*MAX_EVENTS);
}

void serverBase::Work() {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1);
    for(int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        int fd_events = events[i].events;
        if((fd_events & EPOLLERR) || (fd_events & EPOLLHUP) || (!(fd_events & EPOLLIN))) {
            printf("fd:%d error \n", fd);
            HandleClose(fd);
            /*
            clientDatas.clientData.erase(fd);
            clientDatas.ClientFDSet.erase(fd);
            clientDatas.ClientFrameData.erase(fd);
            if(clientDatas.FdToUser.find(fd) != clientDatas.FdToUser.end())
            {
                clientDatas.Users.erase(clientDatas.FdToUser[fd]);
                clientDatas.FdToUser.erase(fd);
            }
            */
            close(fd);
            continue;
        }
        if(events[i].data.fd == server_fd) {
            memset(&server_addr, 0, sizeof(server_addr));
            int length;
            int clientFd = accept(server_fd, (struct sockaddr*)&server_addr, (socklen_t *)&length);
            if(clientFd == -1) {
                printf("accpet socket error: errno = %d, (%s)\n", errno,strerror(errno));
                continue;
            }
            printf("====================\n");
            HandleNetIp(inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), clientFd);
            ev.data.fd = clientFd;
            ev.events = EPOLLIN;
            int epoll_ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientFd, &ev);
            if(epoll_ret == -1) {
                printf("epoll_ctl error: errno = %d, (%s)\n", errno, strerror(errno));
                continue;
            }
        }else {
            int clientFd = events[i].data.fd, ret;
            ret = recv(clientFd, bData.GetBuffArray(), BUFF_SIZE, 0);
            if(ret > 0) {
                HandleEvent(clientFd, ret);
            }else if(ret < 0) {
                printf("recv data from fd: %d error, errno = %d, (%s)\n", clientFd, errno, strerror(errno));
                if(errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN) {
                    HandleClose(clientFd);
                    close(clientFd);
                }
            } else {
                printf("socket:%d closed\n", clientFd);
                HandleClose(clientFd);
                int close_ret = close(clientFd);
                if(close_ret == -1) {
                    printf("close socket:%d error: errno %d, (%s)\n", clientFd, errno, strerror(errno));
                }
            }
        }
    }
}

void serverBase::SendDataToClient(int clientFd, int length) {
    int ret = send(clientFd, bData.GetBuffArray(), length, 0);
    if(ret == -1) {
        TRACE_WARN("send to client error : clientFd = %d, errno = %d, (%s)\n", clientFd, errno, strerror(errno));
        printf("send to client error: clientFd = %d, errno = %d, (%s)\n", clientFd, errno, strerror(errno));
        HandleClose(clientFd);
        close(clientFd);
    }
}
