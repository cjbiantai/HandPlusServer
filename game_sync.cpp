#include"game_sync.h"

void GameSync::RecvAndHandle(int sockfd){
	if(Recv(sockfd)<=0){
		return;
    }
	ClientMsg cmsg;
	while(Parse(sockfd,cmsg)>0){
		switch(cmsg.type()){
			case EnterRoom:
				printf("uid: %d roomid: %d connect\n",cmsg.playerinfo().uid(),cmsg.playerinfo().roomid());
				JoinRoom(sockfd,cmsg.playerinfo().uid(),cmsg.playerinfo().roomid());
				break;
			case C2SSync:
                //printf("uid: %d update\n",player[sockfd].uid);
				Update(sockfd,cmsg.input());
				break;
			default:
				return;
		}
	}
}

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
	for(it=room.begin();it!=room.end();it++){
        //printf("room: %d, broadcast\n",it->first);
#ifdef DEBUG
        cout<<"room "<<it->first<<endl;
#endif
		it->second.Broadcast();
    }
}

void GameSync::Exit(int sockfd){
    printf("uid: %d,fd: %d, exit\n",player[sockfd].uid,sockfd);
    if(player[sockfd].uid<0)
        return;
    player[sockfd].online=false;
	int room_id=player[sockfd].room_id;
	if(!room_id){
		player.erase(sockfd);
		return;
	}
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

int GameSync::Parse(int sockfd,ClientMsg &cmsg){
	if(!player.count(sockfd)){
		printf("player don't exist\n");
		return -1;
	}
	int ret=player[sockfd].Parse(cmsg);
	if(ret<0){
		printf("uid: %d, kick off by server\n",player[sockfd].uid);
		Exit(sockfd);
	}
	return ret;
}

void GameSync::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return;
	return player[sockfd].Update(input);
}

void GameSync::JoinRoom(int sockfd,int uid,int room_id){
#ifdef DEBUG
    cout<<"GameSync::JoinRoom"<<endl;
#endif
    if(room_id<=0){
        printf("uid: %d,room_id: %d,room_id must above zero!\n",uid,room_id);
        SocketError::Check(-1,sockfd);
        return;
    }
	if(uid2room[uid]){
        Player *p=room[uid2room[uid]].GetPlayer(uid);
        if(p->online)
            Exit(p->sockfd);
        if(uid2room[uid]==room_id){
		    player[sockfd].JoinRoom(uid,room_id);
		    room[room_id].Reconnect(uid,&player[sockfd]);
		    return;
        }
	}
	if(!room.count(room_id))
		room[room_id]=Room(ROOM_MAX);
    if(room[room_id].state||room[room_id].max==room[room_id].players.size()){
        printf("uid: %d, can't join room %d!\n",uid,room_id);
        return;
    }
	player[sockfd].JoinRoom(uid,room_id);
	room[room_id].AddPlayer(&player[sockfd]);
	uid2room[uid]=room_id;
}

