#include"player.h"

char Player::sendbuf[BUFFER_SIZE]={};

Player::Player(int sockfd){
	len=0;
	this->sockfd=sockfd;
}

void Player::JoinRoom(string name,int room_id){
	this->name=name;
	this->room_id=room_id;
	online=true;
}

int Player::SendMsg(ServerMsg smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=(len>>24)&0xff;
	sendbuf[2]=(len>>16)&0xff;
	sendbuf[3]=(len>>8)&0xff;
	sendbuf[4]=len&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	return send(sockfd,sendbuf,len+HEADER_LEN,0);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
	int ret=recv(sockfd,buffer+len,BUFFER_SIZE-len,MSG_DONTWAIT);
	if(SocketError::Check(ret,sockfd)<=0)
		return ret;
	len+=ret;
	return 1;
}

bool Player::Parse(ClientMsg &cmsg){
	if(len<HEADER_LEN)
		return NULL;
	int msg_len=(buffer[1]<<24)+(buffer[2]<<16)+(buffer[3]<<8)+buffer[4];
	if(msg_len+HEADER_LEN>BUFFER_SIZE){
		printf("size error\n");
		memset(buffer,len=0,sizeof(buffer));
		return false;
	}
	if(len<msg_len+HEADER_LEN)
		return false;
	int ret=cmsg.ParseFromArray(buffer+HEADER_LEN,msg_len);
	if(!ret){
		printf("parse error\n");
		memset(buffer,len=0,sizeof(buffer));
		return false;
	}
	memcpy(buffer,buffer+HEADER_LEN+msg_len,len-=HEADER_LEN+msg_len);
	return true;
}
