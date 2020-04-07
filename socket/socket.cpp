#include "socket.h"

socketBase::socketBase(int port) {
    this -> port = port;
    Init();
    InitEpoll();
}

socketBase::~socketBase() {
    free(events);
}

void socketBase::Init() {
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

void socketBase::InitEpoll() {
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

