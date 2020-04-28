#include"game_sync.h"

GameSync::GameSync(){
    hall=NULL;
}

GameSync::~GameSync(){
    if(hall!=NULL)
        delete hall;
}

void GameSync::RecvAndHandle(int sockfd){
	if(Recv(sockfd)<=0){
        SocketError::Close(sockfd,"GameSync::RecvAndHandle");
		return;
    }
	ClientMsg cmsg;
	while(Parse(sockfd,&cmsg)>0){
		switch(cmsg.type()){
			case EnterRoom:
				printf("uid: %d, fd: %d, room_id: %d JoinRoom\n",cmsg.playerinfo().userid(),sockfd,cmsg.playerinfo().roomid());
				JoinRoom(sockfd,cmsg.playerinfo().userid(),cmsg.playerinfo().roomid(),cmsg.roominfo().maxplayers());
				break;
			case C2SSync:
                //printf("uid: %d update\n",player[sockfd].uid);
				Update(sockfd,cmsg.input());
				break;
			default:
                printf("RecvAndHandle undefined message type, fd: %d\n",sockfd);
				return;
		}
	}
}

void GameSync::S2SRecvAndHandle(int sockfd){
    if(hall==NULL)
        hall=new HallSync(sockfd);
	if(hall->Recv()<=0){
        printf("hallfd recv error\n");
		return;
    }
	S2SMsg msg;
	while(hall->Parse(&msg)>0){
		switch(msg.type()){
			case PrepareRoom:
                room[msg.roominfo().roomid()]=Room(msg.roominfo().maxplayers());
                hall->SendMsg(new S2SMsg());
				break;
			default:
                printf("S2S undefined message type, fd: %d\n",sockfd);
				return;
		}
	}
    
}

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
	for(it=room.begin();it!=room.end();it++){
		it->second.Broadcast();
    }
}

void GameSync::Exit(int sockfd){
    Player *p_player=&player[sockfd];
    printf("uid: %d,fd: %d, exit\n",p_player->uid,sockfd);
    if(p_player->uid<0){
	    player.erase(sockfd);
        return;
    }
	if(p_player->room_id){
		room[p_player->room_id].DeletePlayer(p_player->uid);
        if(room[p_player->room_id].players.size()==0)
            room.erase(p_player->room_id);
    }
	player.erase(sockfd);
}

void GameSync::Print(){
    for(auto &r:room){
        printf("room: %d\n",r.second.players[0]->room_id);
        for(auto &p:r.second.players)
            printf("    uid: %d, fd: %d, room_id: %d\n",p->uid,p->sockfd,p->room_id);
    }
}

int GameSync::GetRoomId(int sockfd){
	return player[sockfd].room_id;
}

int GameSync::Recv(int sockfd){
	if(!player.count(sockfd)){
        printf("fd: %d connect\n",sockfd);
		player[sockfd]=Player(sockfd);
	}
	return player[sockfd].Recv();
}

int GameSync::Parse(int sockfd,ClientMsg *cmsg){
	if(!player.count(sockfd)){
		printf("player don't exist\n");
		return -1;
	}
	int ret=player[sockfd].Parse(cmsg);
	if(ret<0){
		printf("uid: %d, kick off by server\n",player[sockfd].uid);
		SocketError::Close(sockfd,"GameSync::Parse");
	}
	return ret;
}

void GameSync::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return;
	player[sockfd].Update(input);
}

void GameSync::JoinRoom(int sockfd,int uid,int room_id,int room_max){
#if DEBUG>1
    cout<<"GameSync::JoinRoom"<<endl;
#endif
    if(room_id<=0||uid<=0){
        printf("uid: %d, fd: %d, room_id: %d,room_id and uid must above zero!\n",uid,sockfd,room_id);
        SocketError::Close(sockfd,"GameSync::JoinRoom");
        return;
    }
    int last_room=uid2room[uid].first;
	if(last_room&&room.count(last_room)){
        Player *p_last=room[last_room].GetPlayer(uid);
        if(last_room==room_id&&uid2room[uid].second==room[room_id].timestamp){
            //cout<<uid2room[uid].second<<" "<<room[room_id].timestamp<<endl;
		    player[sockfd].JoinRoom(uid,room_id);
		    room[room_id].Reconnect(uid,&player[sockfd]);
		    return;
        }
        if(p_last!=NULL)
            SocketError::Close(p_last->sockfd,"GameSync::JoinRoom");
	}
	if(!room.count(room_id)){
        if(room_max<=0){
            printf("room_max must above zero!\n");
            SocketError::Close(sockfd,"GameSync::JoinRoom");
            return;
        }
        printf("Room create, room_id: %d, room_max: %d\n",room_id,room_max);
		room[room_id]=Room(room_max);
    }
    if(room[room_id].state||room[room_id].max==room[room_id].players.size()){
        printf("uid: %d, can't join room %d!\n",uid,room_id);
        return;
    }
	player[sockfd].JoinRoom(uid,room_id);
	room[room_id].AddPlayer(&player[sockfd]);
	uid2room[uid]={room_id,room[room_id].timestamp};
}

