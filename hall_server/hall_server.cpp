#include "hall_server.h"

hallServer::hallServer(int port) : socketBase(port) {
}

void hallServer::Work() {
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
            /*
            if(clientDatas.clientData.find(client_fd) == clientDatas.clientData.end()) {
                clientDatas.clientData[client_fd] = RecvDataManager();
            }
            HandleRecvData(client_fd);
            */
        }
    }

}
