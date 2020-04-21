#include"socket_error.h"

int SocketError::epfd=-1;

ServerSync* SocketError::sync=NULL;

int SocketError::Check(int ret,int sockfd){
	if(epfd==-1||sync==NULL){
		printf("fd: %d, epfd or sync not set\n",sockfd);
		return -1;
	}
	if(ret>0)
		return 1;
	if(ret<0){
		printf("fd: %d, recv or send fail: %s\n",sockfd,strerror(errno));
       	if(errno==EINTR||errno==EWOULDBLOCK||errno==EAGAIN)
       		return 0;
       	else{
       		if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
       			printf("fd: %d, epoll delete socket error\n",sockfd);
       		if(close(sockfd)==-1)
       			printf("fd: %d, close socket error: %s\n",sockfd,strerror(errno));
       		sync->Exit(sockfd);
       		return -1;
       	}
	}
    if(ret==0){
	    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
		  	printf("fd: %d, epoll delete socket error\n",sockfd);
		if(close(sockfd)==-1)
		    printf("fd: %d, close socket error: %s\n",sockfd,strerror(errno));
		sync->Exit(sockfd);
		return 0;
	}
}
