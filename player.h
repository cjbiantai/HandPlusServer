#pragma once
#include"common/all.h"
#include"socket_error.h"

class Player{
	private:
		static char sendbuf[BUFFER_SIZE];
		int len;
		char buffer[BUFFER_SIZE<<1];
	public:
		int uid,room_id,sockfd;
		string name;
		PlayerInput input;
		
		Player(){uid=-1;}
		Player(int sockfd);
		void JoinRoom(int uid,int room_id);
		int SendMsg(const ServerMsg &smsg);
		void Update(PlayerInput input);
		int Recv();
		int Parse(ClientMsg &cmsg);
        void ReconnectFail();
};

