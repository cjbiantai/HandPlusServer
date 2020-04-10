#pragma once
#include"common/all.h"
#include"Player.h"

class Room{
	private:
		static char sendbuf[BUFFER_SIZE];
		vector<Player&> players;
		vector<ServerMsg> frames;
	public:
		int max,state;
		Room(int max=2);
		void AddPlayer(Player player);
		int SendToAll(ServerMsg smsg);
		void Broadcast();
		void Retransmission(int sockfd,int beg_fid);
};
