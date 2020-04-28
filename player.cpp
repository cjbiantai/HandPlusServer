#include"player.h"

char Player::sendbuf[BUFFER_SIZE]={};

Player::Player(){
    uid=-1;
}

Player::Player(int sockfd){
    this->sockfd=sockfd;
    socketConn.Init(sockfd);
    uid=-1;
}

void Player::JoinRoom(int uid,int room_id){
#if DEBUG>1
    cout<<"Player::JoinRoom"<<endl;
#endif
	this->uid=uid;
	this->room_id=room_id;
}

int Player::SendMsg(ServerMsg *smsg){
#if DEBUG>1
    cout<<"Player::SendMsg"<<endl;
#endif
    return socketConn.SendMsg(smsg);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
#if DEBUG>1
    cout<<"Player::Recv"<<endl;
#endif
    return socketConn.Recv();
}

int Player::Parse(ClientMsg *cmsg){
    return socketConn.Parse(cmsg);
}

void Player::ReconnectFail(){
}

