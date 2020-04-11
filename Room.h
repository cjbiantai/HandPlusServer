#pragma once
#include"common/all.h"
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
		void Reconnect(string name,Player *player);
		void SendToAll(ServerMsg smsg);
		void Broadcast();
		void Retransmission(int sockfd,int beg_fid);
};

