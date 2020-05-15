#include"socket_connect.h"

unsigned char SocketConnect::sendbuf[BUFFER_SIZE]={};

SocketConnect::SocketConnect(){
    sockfd=-1;
    len=0;
}

SocketConnect::SocketConnect(int sockfd){
    Init(sockfd);
}

void SocketConnect::Init(int sockfd){
    this->sockfd=sockfd;
    len=0;
}

int SocketConnect::Recv(){
    LOG(2,"SocketConnect::Recv");
	int ret=recv(sockfd,buffer+len,BUFFER_SIZE-len,MSG_DONTWAIT);
	if(ret<=0)
        LOG(0,"fd: %d, recv fail",sockfd);
    else
	    len+=ret;
	return ret;
}

int SocketConnect::SendMsg(google::protobuf::Message *msg){
    LOG(2,"SocketConnect::SendMsg");
    if(msg==NULL){
        LOG(0,"SocketConnect::SendMsg msg=NULL");
        return -1;
    }
	int len=msg->ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=len&0xff;
	sendbuf[2]=(len>>8)&0xff;
	sendbuf[3]=(len>>16)&0xff;
	sendbuf[4]=(len>>24)&0xff;
	msg->SerializeToArray(sendbuf+HEADER_LEN,len);
	return send(sockfd,sendbuf,len+HEADER_LEN,0);
}

int SocketConnect::Parse(google::protobuf::Message *msg){ 
    LOG(2,"SocketConnect::Parse");
	if(len<HEADER_LEN||msg==NULL)
		return 0;
	int msg_len=(buffer[4]<<24)+(buffer[3]<<16)+(buffer[2]<<8)+buffer[1];
	if(msg_len<0||msg_len+HEADER_LEN>BUFFER_SIZE){
		LOG(0,"fd: %d, message length error, msg_len=%d",sockfd,msg_len);
		return -1;
	}
	if(len<msg_len+HEADER_LEN)
		return 0;
	int ret=msg->ParseFromArray(buffer+HEADER_LEN,msg_len);
	if(!ret){
		LOG(0,"fd: %d, parse error",sockfd);
		return -1;
	}
	memmove(buffer,buffer+HEADER_LEN+msg_len,len-=HEADER_LEN+msg_len);
	return 1;
}

