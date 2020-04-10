#pragma once

class Player{
	private:
		int len,room_id,sockfd;
		char buffer[SIZE<<1];
		string name;
	public:
		bool online,updated;
		PlayInput input;
		
		int SendMsg(ServerMsg smsg);
		void Update(PlayerInput input);
		int Recv();
		ClientMsg Parse();
};
