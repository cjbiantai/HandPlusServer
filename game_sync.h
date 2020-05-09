#pragma once
#include"common/all.h"
#include"server_sync.h"
#include"socket_error.h"
#include"hall_sync.h"
#include"player.h"
#include"room.h"

class GameSync:public ServerSync{
	private:
        int len;
		map<int,Player> player;	//fd2player
		map<int,Room> room;
		map<int,pii> uid2room;
        HallSync *hall;
	public:
        GameSync();
        ~GameSync();

		void RecvAndHandle(int sockfd);
        void S2SRecvAndHandle(int sockfd);
		void Broadcast();
		void Exit(int sockfd);
        void Print();
		
		int GetRoomId(int sockfd);

		int Recv(int sockfd);
        bool S2SParse(int sockfd,S2SMsg *msg);
		int Parse(int sockfd,ClientMsg *cmsg);
		void Update(int sockfd,PlayerInput input);
		void Reconnect(int sockfd);
		
		void JoinRoom(int sockfd,int uid,int room_id);
		void InitRoom(int roomId);
};

