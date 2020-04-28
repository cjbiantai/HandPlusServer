#include"socket_error.h"

int SocketError::epfd=-1;

ServerSync* SocketError::sync=NULL;

int SocketError::Check(int ret,int sockfd,const char *str){
	if(epfd==-1||sync==NULL){
		printf("fd: %d, epfd or sync not set, in %s\n",sockfd,str);
		return -1;
	}
	if(ret>0)
		return 1;
	if(ret<0){
		printf("fd: %d, recv or send fail: %s, in %s\n",sockfd,strerror(errno),str);
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
    //cout<<"SocketError::Close"<<endl;
    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
        printf("fd: %d, epoll delete socket error, in %s\n",sockfd,str);
    if(close(sockfd)==-1)
       	printf("fd: %d, close socket error: %s, in %s\n",sockfd,strerror(errno),str);
    sync->Exit(sockfd);
}
