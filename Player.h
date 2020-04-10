#pragma once

class Player{
	private:
		int len;
		char buffer[SIZE<<1];		
	public:
		bool online;
		int room_id,sockfd;
		string name;
		PlayInput input;
		
		Player(int sockfd,string name,int room_id);
		int SendMsg(ServerMsg smsg);
		void Update(PlayerInput input);
		int Recv();
		ClientMsg Parse();
};
