#include"Player.h"

Player::Player(int sockfd,string name,int room_id){
	len=0;
	this.sockfd=sockfd;
	this.name=name;
	this.room_id=room_id;
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
	this.input=input;
}

int Player::Recv(){
	int ret;
	if(ret=recv(sockfd,buffer+len,SIZE-len,MSG_DONTWAIT),sockfd<=0)
		return ret;
	len+=ret;
	return 1;
}

ClientMsg Player::Parse(){
	if(len<HEADER_LEN)
		return NULL;
	int msg_len=(buffer[i+1]<<24)+(buffer[i+2]<<16)+(buffer[i+3]<<8)+buffer[i+4];
	if(msg_len+HEADER_LEN>SIZE){
		printf("size error\n");
		memset(buffer,len=0,sizeof(buffer));
		return NULL;
	}
	if(len<msg_len+HEADER_LEN)
		return NULL;
	ClientMsg cmsg;
	int ret=cmsg.ParseFromArray(buffer+HEADER_LEN,msg_len);
	if(!ret){
		printf("parse error\n");
		memset(buffer,len=0,sizeof(buffer));
		return NULL;
	}
	memcpy(buffer,buffer+HEADER_LEN+msg_len,len-=HEADER_LEN+msg_len);
	return cmsg;
}
