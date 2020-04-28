#include"player.h"

char Player::sendbuf[BUFFER_SIZE]={};

Player::Player(int sockfd){
    socketConn=new SocketConnect(sockfd);
    uid=-1;
}

Player::~Player(){
    if(socketConn!=NULL)
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
#ifdef DEBUG
    cout<<"Player::SendMsg"<<endl;
#endif
    return socketConn->SendMsg(smsg);
}

void Player::Update(PlayerInput input){
	this->input=input;
}

int Player::Recv(){
#ifdef DEBUG
    cout<<"Player::Recv"<<endl;
#endif
    return socketConn->Recv();
}

int Player::Parse(ClientMsg *cmsg){
    return socketConn->Parse(cmsg);
}

void Player::ReconnectFail(){
}

