#pragma once
#include"common/all.h"
#include"socket_error.h"
#include"player.h"

class Room{
	private:
		static char sendbuf[BUFFER_SIZE];
		vector<Player*> players;
		vector<ServerMsg> frames;
	public:
		int max,state;
		Room(){}
		Room(int max);
		void AddPlayer(Player *player);
		void Reconnect(int uid,Player *player);
		void SendToAll(ServerMsg smsg);
		void Broadcast();
};

