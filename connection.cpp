#include"connection.h"

void SendMsg(int sockfd,ServerMsg smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=(len>>24)&0xff;
	sendbuf[2]=(len>>16)&0xff;
	sendbuf[3]=(len>>8)&0xff;
	sendbuf[4]=len&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	check(send(sockfd,sendbuf,len+HEADER_LEN,0),sockfd);
}

void SendToRoom(int roomId,ServerMsg smsg){
	int len=smsg.ByteSize();
	sendbuf[0]=1;
	sendbuf[1]=(len>>24)&0xff;
	sendbuf[2]=(len>>16)&0xff;
	sendbuf[3]=(len>>8)&0xff;
	sendbuf[4]=len&0xff;
	smsg.SerializeToArray(sendbuf+HEADER_LEN,len);
	int sockfd;
	for(int i=0;i<room[roomId].sockfd.size();i++){
		sockfd=connManager.room[roomId].sockfd[i];
		check(send(sockfd,sendbuf,len+HEADER_LEN,0),sockfd);
	}	
}

void ConnectionManager::RoomInit(int roomId){
	room[roomId].state=1;
	ServerMsg smsg;
	InputList *p_input;
	Position *p_pos;
	PlayerRecord *rec;
	string name,str,*p_list;
	for(int i=0;i<room[roomId].sockfd.size();i++){
		name=sockstate[room[roomId].sockfd[i]].name;
		PlayerInput tmp;
		tmp.set_name(name);
		p_pos=tmp.mutable_pos();
		rec=&table[name];
		p_pos->set_x(rec->x);
		p_pos->set_y(rec->y);
		p_pos->set_z(rec->z);
		tmp.SerializeToString(&str);
		p_input=smsg.add_inputs();
		p_list=p_input->add_list();
		*p_list=str;
	}
	smsg.set_code(1);
	smsg.set_fid(0);
	room[roomId].frames.push_back(smsg);
	SendToRoom(roomId,smsg);
}

void ConnectionManager::JoinRoom(int sockfd,int roomId){
	ServerMsg smsg;
	if(!room.count(roomId)){
		room[roomId]=Room();
		room[roomId].max=2;
	}
	if(room[roomId].state){
		smsg.set_code(1);
		smsg.set_str("room is full");
		SendMsg(sockfd,smsg);
		return;
	}
	room[roomId].sockfd.push_back(sockfd);
	sockstate[sockfd].roomId=roomId;
	name2room[sockstate[sockfd].name]=roomId;
	smsg.set_code(0);
	smsg.set_str("join room success");
	//SendMsg(sockfd,smsg);
	if(room[roomId].max==room[roomId].sockfd.size())
		RoomInit(roomId);
}

void ConnectionManager::Exit(int sockfd){
	int roomId=sockstate[sockfd].roomId;
	if(!roomId){
		sockstate.erase(sockfd);
		return;
	}
	room[roomId].state=0;
	vector<int>::iterator it;
	for(it=room[roomId].sockfd.begin();it!=room[roomId].sockfd.end();it++)
		if(*it==sockfd){
			room[roomId].sockfd.erase(it);
			break;
		}
	sockstate.erase(sockfd);
}

bool ConnectionManager::isOnline(int sockfd){
	return sockstate[sockfd].online;
}

int ConnectionManager::GetRoomId(int sockfd){
	return sockstate[sockfd].roomId;
}
