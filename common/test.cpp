#include"../common/all.h"

#define SIZE 1024

void SendMsg(int sockfd,ClientMsg cmsg){
	char sendbuf[SIZE]={};
	int len=cmsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[4]=(len>>24)&0xff;
	sendbuf[3]=(len>>16)&0xff;
	sendbuf[2]=(len>>8)&0xff;
	sendbuf[1]=len&0xff;
	cmsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	send(sockfd,sendbuf,len+HEADER_LEN,0);
}

int main(){
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("117.78.9.170");
    saddr.sin_port = htons(17000);
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0)
    	return 0*printf("connect fail\n");
    ClientMsg cmsg;
    PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
    playerinfo->set_account("a");
    playerinfo->set_password("a");
    for(int i=1;i<=10000;i++)
    	SendMsg(sockfd,cmsg);
    close(sockfd);
    
	return 0;
}
