#pragma once
#include"Player.h"
#include"Room.h"

class GameManager{
	private:
		map<int,Player> player;	//fd2player
		map<string,int> name2room;
		map<int,Room> room;
	public:
		bool isOnline(int sockfd);
		int GetRoomId(int sockfd);
		int Check(int ret,int sockfd);

		void Recv(int sockfd);
		ClientMsg Parse(int sockfd);
		void Update(PlayerInput input);
		void Retransmission(int sockfd,int beg_fid);
		
		void JoinRoom(int sockfd,int roomId);
		void Exit(int sockfd);
		void InitRoom(int roomId);
}gameManager;
