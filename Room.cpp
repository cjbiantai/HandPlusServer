#include"Room.h"

Room::Room(int max=2){
	state=0;
	this.max=max;
}

void Room::AddPlayer(Player player){
	players.push_back(player);
	if(players.size()==max){
		state=1;
		//SendToAll(smsg); smsg game start state
	}
}

void Room::SendToAll(ServerMsg smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=(len>>24)&0xff;
	sendbuf[2]=(len>>16)&0xff;
	sendbuf[3]=(len>>8)&0xff;
	sendbuf[4]=len&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	int ret;
	for(int i=0;i<players.size();i++){
		ret=send(players[i].sockfd,sendbuf,len+HEADER_LEN,0);
		if(ret<=0)
			continue;
	}
}

void Room::Broadcast(){
	if(!state)
		return;
	ServerMsg frame;
	frame.set_code(3);
	frame.set_fid(frames.size());
	PlayerInput *input;
	for(int i=0;i<players.size();i++){
		//if(!players[i].updated)
		//	duplicate(players[i].input);
		//players[i].updated=false;
		input=frame.add_inputs();
		*input=players[i].input;
	}
	frames.push_back(frame);
	SendToAll(frame);
}

void Room::Retransmission(int sockfd,int beg_fid){
	Player &player;
	for(int i=0;i<players.size();i++)
		if(players[i].sockfd==sockfd){
			player=players[i];
			break;
		}
	if(player.SendMsg(frames[beg_fid])<=0)
		return;
	for(int i=beg_fid;i<frames.size();i++)
		if(player.SendMsg(frames[i])<=0)
			return;///////////////////////////check
}
