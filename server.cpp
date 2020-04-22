#include "server.h"

Server::Server(int port,ServerSync *sync) {
    this->port=port;
    this->sync=sync;
    tick=0;
    
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
    	printf("socket error: %d\n", errno);
    	exit(0);
    }
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=htonl(INADDR_ANY);
    saddr.sin_port=htons(port);
    int value=1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&value, sizeof(value));
    setsockopt(listenfd,IPPROTO_TCP,TCP_NODELAY,(void*)&value,sizeof(value));
    if(bind(listenfd,(struct sockaddr*)&saddr,sizeof(saddr))<0){
    	printf("bind error: %d\n",errno);
    	if(close(listenfd)==-1)
		    printf("close socket error: %d\n",errno);
    	exit(0);
    }
    if(listen(listenfd,128)<0){
		printf("listen error:%d\n",errno);
		if(close(listenfd)==-1)
		    printf("close socket error: %d\n",errno);
    	exit(0);
    }
	SocketError::epfd=epfd=epoll_create(MAX_EVENTS);
	struct epoll_event event;
	event.data.fd=listenfd;
	event.events=EPOLLIN;
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&event)<0){
		printf("epoll_ctl listenfd error\n");
		exit(0);
	}
	events=(struct epoll_event*)malloc(sizeof(event)*MAX_EVENTS);
}

Server::~Server() {
    free(events);
    close(epfd);
}

void Server::WorkOnce() {
	struct sockaddr caddr;
    socklen_t len;
    int sockfd;
   	gettimeofday(&start,NULL);
	int n=epoll_wait(epfd,events,MAX_EVENTS,1);
	struct epoll_event event;
   	for(int i=0;i<n;i++){
   		if(events[i].data.fd==listenfd){
   			len=sizeof(caddr);
   			sockfd=accept(listenfd,&caddr,&len);
   			if(sockfd==-1){
   				printf("accept new sock error: %d\n",errno);
   				continue;
   			}
   			event.data.fd=sockfd;
   			event.events=EPOLLIN;
   			if(epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event)<0){
   				printf("epoll_ctl new sock error,address :%s\n",caddr.sa_data);
   				continue;
   			}
   		}else{
	        sockfd=events[i].data.fd;
	        sync->RecvAndHandle(sockfd);
   		}
   	}
   	gettimeofday(&end,NULL);
   	tick+=(end.tv_sec-start.tv_sec)*1000LL+(end.tv_usec-start.tv_usec)/1000;
   	if(tick>=BROADCAST_RATE){
   		tick-=BROADCAST_RATE;
	   	sync->Broadcast();
	}
}
