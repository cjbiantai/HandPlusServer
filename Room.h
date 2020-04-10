#pragma once
#include"Player.h"

class Room{
	private:
		static char sendbuf[BUFFER_SIZE];
		vector<Player&> players;
		vector<ServerMsg> frames;
	public:
		int max,state;
		void duplicate(InputList &input);
		Room(int max);
		void Broadcast();
		void Retransmission(int sockfd,int beg_fid);
};
