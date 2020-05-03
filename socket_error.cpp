#include"socket_error.h"

int SocketError::epfd=-1;

ServerSync* SocketError::sync=NULL;

int SocketError::Check(int ret,int sockfd,const char *str){
	if(epfd==-1||sync==NULL){
		LOG(0,"fd: %d, epfd or sync not set, in %s",sockfd,str);
		return -1;
	}
	if(ret>0)
		return 1;
	if(ret<0){
		LOG(0,"fd: %d, recv or send fail: %s, in %s",sockfd,strerror(errno),str);
       	if(errno==EINTR||errno==EWOULDBLOCK||errno==EAGAIN)
       		return 0;
       	else{
            Close(sockfd,str);
       		return -1;
       	}
	}
    if(ret==0){
        Close(sockfd,str);
		return 0;
	}
}


void SocketError::Close(int sockfd,const char *str){
    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
        LOG(0,"fd: %d, epoll delete socket error, in %s",sockfd,str);
    if(close(sockfd)==-1)
       	LOG(0,"fd: %d, close socket error: %s, in %s",sockfd,strerror(errno),str);
    sync->Exit(sockfd);
}
