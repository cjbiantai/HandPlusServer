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
    LOG(2,"Player::JoinRoom");
	this->uid=uid;
	this->room_id=room_id;
}

int Player::SendMsg(ServerMsg *smsg){
    LOG(2,"Player::SendMsg");
    return socketConn.SendMsg(smsg);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
    LOG(2,"Player::Recv");
    return socketConn.Recv();
}

int Player::Parse(ClientMsg *cmsg){
    return socketConn.Parse(cmsg);
}

void Player::ReconnectFail(){
}

