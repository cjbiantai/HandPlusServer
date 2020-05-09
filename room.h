#pragma once
#include"common/all.h"
#include"socket_error.h"
#include"player.h"

class Room{
	private:
		static unsigned char sendbuf[BUFFER_SIZE];
        static int __serial_id;
	public:
		int max,state,serial_id,timestamp;
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

