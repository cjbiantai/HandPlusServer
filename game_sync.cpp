#include"game_sync.h"

void GameSync::RecvAndHandle(int sockfd){
	if(Recv(sockfd)<=0)
		return;
	ClientMsg cmsg;
	while(Parse(sockfd,cmsg)){
		switch(cmsg.type()){
			case EnterRoom:
				printf("uid: %d roomid: %d connect\n",cmsg.playerinfo().uid(),cmsg.playerinfo().roomid());
				JoinRoom(sockfd,cmsg.playerinfo().uid(),cmsg.playerinfo().roomid());
				break;
			case C2SSync:
                printf("uid: %d update\n",player[sockfd].uid);
				Update(sockfd,cmsg.input());
				break;
			default:
				return;
		}
	}
}

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
	for(it=room.begin();it!=room.end();it++)
		it->second.Broadcast();
}

void GameSync::Exit(int sockfd){
	int room_id=player[sockfd].room_id;
	if(!room_id){
		player.erase(sockfd);
		return;
	}
	room[room_id].state=0;
}

bool GameSync::isOnline(int sockfd){
	return player[sockfd].online;
}

int GameSync::GetRoomId(int sockfd){
	return player[sockfd].room_id;
}

int GameSync::Recv(int sockfd){
	if(!player.count(sockfd)){
		player[sockfd]=Player(sockfd);
	}
	return player[sockfd].Recv();
}

bool GameSync::Parse(int sockfd,ClientMsg &cmsg){
	if(!player.count(sockfd))
		return false;
	return player[sockfd].Parse(cmsg);
}

void GameSync::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return;
	return player[sockfd].Update(input);
}

void GameSync::JoinRoom(int sockfd,int uid,int room_id){
	if(uid2room[uid]){
		player[sockfd].JoinRoom(uid,room_id);
		room[uid2room[uid]].Reconnect(uid,&player[sockfd]);
		return;
	}
	if(!room.count(room_id))
		room[room_id]=Room(ROOM_MAX);
	player[sockfd].JoinRoom(uid,room_id);
	uid2room[uid]=room_id;
	room[room_id].AddPlayer(&player[sockfd]);
}


