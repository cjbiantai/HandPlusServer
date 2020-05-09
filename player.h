#pragma once
#include"common/all.h"
#include"socket_error.h"
#include"socket_connect.h"

class Player{
	private:
		static unsigned char sendbuf[BUFFER_SIZE];
		int len;
		char unsigned buffer[BUFFER_SIZE<<1];
	public:
        SocketConnect socketConn;
		int uid,room_id,sockfd;
		string name;
		PlayerInput input;
		
        Player();
        Player(int sockfd);
		void JoinRoom(int uid,int room_id);
		int SendMsg(ServerMsg *smsg);
		void Update(PlayerInput input);
		int Recv();
		int Parse(ClientMsg *cmsg);
        void ReconnectFail();
};

