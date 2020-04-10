#pragma once
#include"common/all.h"

class Player{
	private:
		int len;
		char buffer[BUFFER_SIZE<<1];
	public:
		bool online;
		int room_id,sockfd;
		string name;
		PlayerInput input;
		
		Player(int sockfd,string name,int room_id);
		int SendMsg(ServerMsg smsg);
		void Update(PlayerInput input);
		int Recv();
		ClientMsg Parse();
};
