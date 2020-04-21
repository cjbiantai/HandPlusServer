#pragma once
#include"common/all.h"
#include"server_sync.h"
#include"socket_error.h"
#include"player.h"
#include"room.h"

class GameSync:public ServerSync{
	private:
		char buffer[BUFFER_SIZE];
		char sendbuf[BUFFER_SIZE];
		map<int,Player> player;	//fd2player
		map<int,int> uid2room;
		map<int,Room> room;
	public:
		void RecvAndHandle(int sockfd);
		void Broadcast();
		void Exit(int sockfd);
		
		bool isOnline(int sockfd);
		int GetRoomId(int sockfd);

		int Recv(int sockfd);
		int Parse(int sockfd,ClientMsg &cmsg);
		void Update(int sockfd,PlayerInput input);
		void Reconnect(int sockfd);
		
		void JoinRoom(int sockfd,int uid,int room_id);
		void InitRoom(int roomId);
};
