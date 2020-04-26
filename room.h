#pragma once
#include"common/all.h"
#include"socket_error.h"
#include"player.h"

class Room{
	private:
		static char sendbuf[BUFFER_SIZE];
        static int serial_id;
	public:
		int max,state,timestamp;
		vector<Player*> players;
		vector<ServerMsg> frames;
		Room(){}
		Room(int max);
		void AddPlayer(Player *player);
        Player* GetPlayer(int uid);
        void DeletePlayer(int uid);
		void Reconnect(int uid,Player *player);
		void SendToAll(ServerMsg smsg);
		void Broadcast();
};

