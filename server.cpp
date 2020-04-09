#include "socket.h"

Server::Server(int port,ServerSync sync) {
    this -> port = port;
    this -> sync = sync;
    Init();
    InitEpoll();
}

Server::~Server() {
    free(events);
}

void Server::Init() {
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
    setsockopt(server_fd,IPPROTO_TCP,TCP_NODELAY,(void*)&value,sizeof(value));

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

void Server::InitEpoll() {
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

void Server::WorkOnce() {
    gettimeofday(&start,NULL);
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1);
    for(int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        int fd_events = events[i].events;
        if((fd_events & EPOLLERR) || (fd_events & EPOLLHUP) || (!(fd_events & EPOLLIN))) {
            printf("fd:%d error \n", fd);
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
            int client_fd = accept(server_fd, (struct sockaddr*)NULL, NULL);
            if(client_fd == -1) {
                printf("accpet socket error: errno = %d, (%s)\n", errno,strerror(errno));
            }
            ev.data.fd = client_fd;
            ev.events = EPOLLIN;
            int epoll_ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            if(epoll_ret == -1) {
                printf("epoll_ctl error: errno = %d, (%s)\n", errno, strerror(errno));
                continue;
            }
        }else {
            int client_fd = events[i].data.fd;
            dataHandler.RecvHandler(client_fd);
            sync.RecvAndHandler();
            /*
            if(clientDatas.clientData.find(client_fd) == clientDatas.clientData.end()) {
                clientDatas.clientData[client_fd] = RecvDataManager();
            }
            HandleRecvData(client_fd);
            */
        }
    }
    gettimeofday(&end,NULL);
    tick+=(end.tv_sec-start.tv_sec)*1000LL+(end.tv_usec-start.tv_usec)/1000;
    if(tick>=BROADCAST_RATE){
    	tick-=BROADCAST_RATE;
    	dataHandler.Broadcast();
    }
}
