#include"player.h"

char Player::sendbuf[BUFFER_SIZE]={};

Player::Player(int sockfd){
    socketConn=new SocketConnect(sockfd);
    uid=-1;
}

Player::~Player(){
    delete socketConn;
}

void Player::JoinRoom(int uid,int room_id){
#ifdef DEBUG
    cout<<"Player::JoinRoom"<<endl;
#endif
	this->uid=uid;
	this->room_id=room_id;
}

int Player::SendMsg(ServerMsg *smsg){
    return socketConn->SendMsg(smsg,0);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
    return socketConn->Recv();
}

int Player::Parse(ClientMsg *cmsg){
    return socketConn->Parse(cmsg);
}

void Player::ReconnectFail(){
}

