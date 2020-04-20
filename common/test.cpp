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

bool RecvMsg(int sockfd,ServerMsg &smsg){
	char buf[SIZE]={};
	if(recv(sockfd,buf,SIZE,0)<=0){
		printf("fd: %d, recv fail\n");
		return false;
	}
	int len=(buf[4]<<24)+(buf[3]<<16)+(buf[2]<<8)+buf[1];
	if(!smsg.ParseFromArray(buf+5,len)){
		printf("fd: %d, parse fail\n");
		return false;
	}
	return true;
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
    ServerMsg smsg;
    cmsg.set_type(LogIn);
    PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
    playerinfo->set_password("toad");
    playerinfo->set_nickname("toad");
    for(int i=0;i<=9999;i++){
    	string account="toad_"+to_string(i);
    	playerinfo->set_account(account);
    	SendMsg(sockfd,cmsg);
    	if(RecvMsg(sockfd,smsg))
    		cout<<smsg.type()<<endl;
    }
    close(sockfd);
    
	return 0;
}
