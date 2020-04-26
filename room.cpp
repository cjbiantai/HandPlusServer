#include"room.h"

char Room::sendbuf[BUFFER_SIZE]={};

Room::Room(int max){
	state=0;
	this->max=max;
}

void Room::AddPlayer(Player *player){
#ifdef DEBUG
    cout<<"Room::AddPlayer uid: "<<player->uid<<endl;
#endif
	players.push_back(player);
	if(players.size()==max){
		state=1;
		//SendToAll(smsg); smsg game start state
	}
}

void Room::DeletePlayer(int uid){
    for(int i=0;i<players.size();i++)
        if(players[i]->uid==uid){
            swap(players[i],players[players.size()-1]);
            players.pop_back();
            return;
        }
}

Player* Room::GetPlayer(int uid){
    for(int i=0;i<players.size();i++)
        if(players[i]->uid==uid)
            return players[i];
    return NULL;
}

void Room::Reconnect(int uid,Player *player){
    printf("uid: %d, Room::Reconnect\n",uid);
    players.push_back(player);
    if(state==2){
        printf("Room Reconnect fail, beyond 1 min limit\n");
        player->ReconnectFail();
        return;
    }
	for(int i=0;i<frames.size();i++)
		if(SocketError::Check(player->SendMsg(frames[i]),player->sockfd)<=0){
            printf("Room::Reconnect fail, send error\n");
			return;
        }
}

void Room::SendToAll(ServerMsg smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=len&0xff;
	sendbuf[2]=(len>>8)&0xff;
	sendbuf[3]=(len>>16)&0xff;
	sendbuf[4]=(len>>24)&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	int ret;
	for(int i=0;i<players.size();i++){
#ifdef DEBUG
        printf("    uid: %d, fd: %d, room_id: %d\n",players[i]->uid,players[i]->sockfd,players[i]->room_id);
#endif
		ret=send(players[i]->sockfd,sendbuf,len+HEADER_LEN,0);
	    if(SocketError::Check(ret,players[i]->sockfd)<=0){
            printf("Room::SendToAll fail\n");
	    }
    }
}

void Room::Broadcast(){
	if(!state)
		return;
	ServerMsg frame;
	frame.set_type(S2CSync);
	frame.set_fid(frames.size());
	PlayerInput *input;
	for(int i=0;i<players.size();i++){
		//if(!players[i].updated)
		//	duplicate(players[i].input);
		//players[i].updated=false;
		input=frame.add_inputs();
		*input=players[i]->input;
        input->set_uid(players[i]->uid);
	}
	frames.push_back(frame);
	SendToAll(frame);
    if(frames.size()>=1200){
        frames.clear();
        state=2;
    }
}

