#include"socket_error.h"

int SocketError::epfd=-1;

ServerSync* SocketError::sync=NULL;

int SocketError::Check(int ret,int sockfd){
	if(epfd==-1||sync==NULL){
		printf("epfd or sync not set\n");
		return -1;
	}
	if(ret>0)
		return 1;
	if(ret<0){
		printf("recv or send fail: %d, %s\n",errno,strerror(errno));
       	if(errno==EINTR||errno==EWOULDBLOCK||errno==EAGAIN)
       		return 0;
       	else{
       		if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
       			printf("epoll delete socket error\n");
       		if(close(sockfd)==-1)
       			printf("close socket error: %d, %s\n",errno,strerror(errno));
       		sync->Exit(sockfd);
       		return -1;
       	}
	}
    if(ret==0){
	    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
		  	printf("epoll delete socket error\n");
		if(close(sockfd)==-1)
		    printf("close socket error: %d, %s\n",errno,strerror(errno));
		sync->Exit(sockfd);
		return 0;
	}
}
