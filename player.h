#pragma once
#include"common/all.h"
#include"socket_error.h"

class Player{
	private:
		static char sendbuf[BUFFER_SIZE];
		int len;
		char buffer[BUFFER_SIZE<<1];
	public:
		bool online;
		int room_id,sockfd;
		string name;
		PlayerInput input;
		
		Player(){}
		Player(int sockfd);
		void JoinRoom(string name,int room_id);
		int SendMsg(ServerMsg smsg);
		void Update(PlayerInput input);
		int Recv();
		bool Parse(ClientMsg &cmsg);
};

