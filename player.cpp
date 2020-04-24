#include"player.h"

char Player::sendbuf[BUFFER_SIZE]={};

Player::Player(int sockfd){
	len=0;
	this->sockfd=sockfd;
    uid=-1;
}

void Player::JoinRoom(int uid,int room_id){
#ifdef DEBUG
    cout<<"Player::JoinRoom"<<endl;
#endif
	this->uid=uid;
	this->room_id=room_id;
}

int Player::SendMsg(const ServerMsg &smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=len&0xff;
	sendbuf[2]=(len>>8)&0xff;
	sendbuf[3]=(len>>16)&0xff;
	sendbuf[4]=(len>>24)&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	return send(sockfd,sendbuf,len+HEADER_LEN,0);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
	int ret=recv(sockfd,buffer+len,BUFFER_SIZE-len,MSG_DONTWAIT);
	if(SocketError::Check(ret,sockfd)<=0){
        printf("Player::Recv fail\n");
		return ret;
    }
	len+=ret;
	return 1;
}

int Player::Parse(ClientMsg &cmsg){
	if(len<HEADER_LEN)
		return 0;
	int msg_len=(buffer[4]<<24)+(buffer[3]<<16)+(buffer[2]<<8)+buffer[1];
	if(msg_len<0||msg_len+HEADER_LEN>BUFFER_SIZE){
		printf("uid: %d, fd: %d, message length error\n",uid,sockfd);
		memset(buffer,len=0,sizeof(buffer));
		return -1;
	}
	if(len<msg_len+HEADER_LEN)
		return 0;
	int ret=cmsg.ParseFromArray(buffer+HEADER_LEN,msg_len);
	if(!ret){
		printf("uid: %d, fd: %d, parse error\n",uid,sockfd);
		return -1;
	}
	memcpy(buffer,buffer+HEADER_LEN+msg_len,len-=HEADER_LEN+msg_len);
	return 1;
}

void Player::ReconnectFail(){
    ServerMsg smsg;
    //smsg.set_type(ReconnectFail);
}

