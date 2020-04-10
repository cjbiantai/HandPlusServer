#include"Room.h"

void Room::Broadcast(ServerMsg smsg){
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
