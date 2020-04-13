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
		map<string,int> name2room;
		map<int,Room> room;
	public:
		void RecvAndHandle(int sockfd);
		void Broadcast();
		void Exit(int sockfd);
		
		bool isOnline(int sockfd);
		int GetRoomId(int sockfd);

		bool Recv(int sockfd);
		bool Parse(int sockfd,ClientMsg &cmsg);
		void Update(int sockfd,PlayerInput input);
		void Retransmission(int sockfd,int beg_fid);
		
		void JoinRoom(int sockfd,string name,int room_id);
		void InitRoom(int roomId);
};
