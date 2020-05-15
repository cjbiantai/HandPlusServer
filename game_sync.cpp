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
				LOG(0,"uid: %d, fd: %d, room_id: %d JoinRoom",cmsg.playerinfo().userid(),sockfd,cmsg.playerinfo().roomid());
				JoinRoom(sockfd,cmsg.playerinfo().userid(),cmsg.playerinfo().roomid());
				break;
			case C2SSync:
                //LOG(1,"uid: %d update",player[sockfd].uid);
				Update(sockfd,cmsg.input());
				break;
			default:
                LOG(0,"RecvAndHandle undefined message type, fd: %d",sockfd);
                Exit(sockfd);
				return;
		}
	}
}

void GameSync::S2SRecvAndHandle(int sockfd){
    if(hall==NULL)
        hall=new HallSync(sockfd);
	if(hall->Recv()<=0){
        LOG(0,"hallfd recv error");
        SocketError::Close(sockfd,"GameSync::S2SRecvAndHandle");
		return;
    }
	S2SMsg msg,send_msg;
    send_msg.set_type(PrepareRoom);
    RoomInfo *p_roominfo=send_msg.mutable_roominfo();
	while(hall->Parse(&msg)>0){
		switch(msg.type()){
			case PrepareRoom:
                if(room.count(msg.roominfo().roomid())){
                    LOG(0,"S2SRecvAndHandle room_id: %d exist",msg.roominfo().roomid());
                    break;
                }
                LOG(0,"Room create, room_id: %d, room_max: %d",msg.roominfo().roomid(),msg.roominfo().curplayernumber());
                room[msg.roominfo().roomid()]=Room(msg.roominfo().curplayernumber());
                p_roominfo->set_roomid(msg.roominfo().roomid());
                hall->SendMsg(&send_msg);
				break;
			default:
                LOG(0,"S2S undefined message type, fd: %d",sockfd);
				return;
		}
	}
    
}

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
    Room *p_room;
    ll now_time=time(0);
	for(it=room.begin();it!=room.end();it++){
        p_room=&it->second;
        if(p_room->players.size()==0&&now_time-p_room->timestamp>180)
            room.erase(it);
        else
		    it->second.Broadcast();
    }
}

void GameSync::Exit(int sockfd){
    Player *p_player=&player[sockfd];
    LOG(0,"uid: %d,fd: %d, exit",p_player->uid,sockfd);
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
        if(r.second.players.size()>0)
            LOG(0,"room: %d",r.second.players[0]->room_id);
        for(auto &p:r.second.players)
            LOG(0,"    uid: %d, fd: %d, room_id: %d",p->uid,p->sockfd,p->room_id);
    }
}

int GameSync::GetRoomId(int sockfd){
	return player[sockfd].room_id;
}

int GameSync::Recv(int sockfd){
	if(!player.count(sockfd)){
        LOG(0,"fd: %d connect",sockfd);
		player[sockfd]=Player(sockfd);
	}
	return player[sockfd].Recv();
}

int GameSync::Parse(int sockfd,ClientMsg *cmsg){
	if(!player.count(sockfd)){
		LOG(0,"player don't exist");
		return -1;
	}
	int ret=player[sockfd].Parse(cmsg);
	if(ret<0){
		LOG(0,"uid: %d, kick off by server",player[sockfd].uid);
		SocketError::Close(sockfd,"GameSync::Parse");
	}
	return ret;
}

void GameSync::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return;
	player[sockfd].Update(input);
}

void GameSync::JoinRoom(int sockfd,int uid,int room_id){
    LOG(1,"GameSync::JoinRoom");
    if(room_id<=0||uid<=0){
        LOG(0,"uid: %d, fd: %d, room_id: %d,room_id and uid must above zero!",uid,sockfd,room_id);
        SocketError::Close(sockfd,"GameSync::JoinRoom");
        return;
    }
    int last_room=uid2room[uid].first;
	if(last_room&&room.count(last_room)){
        Player *p_last=room[last_room].GetPlayer(uid);
        if(p_last==&player[sockfd])
            return;
        if(last_room==room_id&&uid2room[uid].second==room[room_id].serial_id){
		    player[sockfd].JoinRoom(uid,room_id);
		    room[room_id].Reconnect(uid,&player[sockfd]);
		    return;
        }
        if(p_last!=NULL)
            SocketError::Close(p_last->sockfd,"GameSync::JoinRoom");
	}
	if(!room.count(room_id)){
        LOG(0,"Room don't exist, uid: %d, sockfd: %d, room_id: %d",uid,sockfd,room_id);
        SocketError::Close(sockfd,"GameSync::JoinRoom");
        return;
    }
    if(room[room_id].state||room[room_id].max==room[room_id].players.size()){
        LOG(0,"uid: %d, can't join room %d!",uid,room_id);
        return;
    }
	player[sockfd].JoinRoom(uid,room_id);
	room[room_id].AddPlayer(&player[sockfd]);
	uid2room[uid]={room_id,room[room_id].serial_id};
}

