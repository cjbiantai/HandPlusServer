#include"game_manager.h"

bool GameManager::isOnline(int sockfd){
	return sockstate[sockfd].online;
}

int GameManager::GetRoomId(int sockfd){
	return sockstate[sockfd].room_id;
}

int GameManager::Check(int ret,int sockfd){
	if(ret>0)
		return 1;
	if(ret<0){
		printf("recv fail: %d\n",errno);
       	if(errno==EINTR||errno==EWOULDBLOCK||errno==EAGAIN)
       		return 0;
       	else{
       		if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
       			printf("delete socket error\n");
       		if(close(sockfd)==-1)
       			printf("close socket error: %d\n",errno);
       		gameManager.Exit(sockfd);
       		return -1;
       	}
	}
    if(ret==0){
	    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
		  	printf("delete socket error\n");
		if(close(sockfd)==-1)
		    printf("close socket error: %d\n",errno);
		gameManager.Exit(sockfd);
		return 0;
	}
}

void GameManager::Recv(int sockfd){
	if(!player.count(sockfd))
		return;
	check(player[sockfd].Recv(),sockfd);
}

ClientMsg Parse(int sockfd){
	if(!player.count(sockfd))
		return NULL;
	return player[sockfd].Parse();
}

void GameManager::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return NULL;
	return player[sockfd].Update(input);
}

void GameManager::Retransmission(int sockfd,int beg_fid){
	room[GetRoomId(sockfd)].Retransmission(sockfd,beg_fid);
}

void GameManager::RoomInit(int room_id){
	room[room_id].state=1;
	ServerMsg smsg;
	InputList *p_input;
	Position *p_pos;
	PlayerRecord *rec;
	string name,str,*p_list;
	for(int i=0;i<room[room_id].players.size();i++){
		name=sockstate[room[room_id].sockfd[i]].name;
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
	room[room_id].frames.push_back(smsg);
	SendToRoom(room_id,smsg);
}

void GameManager::JoinRoom(int sockfd,string name,int room_id){
	if(!room.count(room_id))
		room[room_id]=Room(2);
	player[sockfd]=Player(sockfd,name,room_id);
	name2room[name]=room_id;
	room[room_id].AddPlayer(player[sockfd]);
}

void GameManager::Exit(int sockfd){
	int room_id=sockstate[sockfd].room_id;
	if(!room_id){
		sockstate.erase(sockfd);
		return;
	}
	room[room_id].state=0;
	vector<int>::iterator it;
	for(it=room[room_id].sockfd.begin();it!=room[room_id].sockfd.end();it++)
		if(*it==sockfd){
			room[room_id].sockfd.erase(it);
			break;
		}
	sockstate.erase(sockfd);
}

void GameManager::Broadcast(){
	map<int,Room>::iterator it;
	for(it=room.begin().it!=room.end();it++)
		it->second.Broadcast();
}

