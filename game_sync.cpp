#include"game_sync.h"

void GameSync::RecvAndHandle(int sockfd){
	Recv(sockfd);
	ClientMsg cmsg;
	while(cmsg=gameManager.Parse(sockfd)!=NULL){
		switch(cmsg.type()){
			case 2:
				JoinRoom(sockfd,cmsg.name(),cmsg.id());
				break;
			case 3:
				Update(sockfd,cmsg.input());
				break;
			case 4:
				Retransmission(sockfd,cmsg.id());
				break;
			default:
				return;
		}
	}
}

void GameSync::Broadcast(){
	gameManager.Broadcast();
}

bool GameSync::isOnline(int sockfd){
	return sockstate[sockfd].online;
}

int GameSync::GetRoomId(int sockfd){
	return sockstate[sockfd].room_id;
}

int GameSync::Check(int ret,int sockfd){
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
       		Exit(sockfd);
       		return -1;
       	}
	}
    if(ret==0){
	    if(epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL)<0)
		  	printf("delete socket error\n");
		if(close(sockfd)==-1)
		    printf("close socket error: %d\n",errno);
		Exit(sockfd);
		return 0;
	}
}

void GameSync::Recv(int sockfd){
	if(!player.count(sockfd))
		return;
	Check(player[sockfd].Recv(),sockfd);
}

ClientMsg Parse(int sockfd){
	if(!player.count(sockfd))
		return NULL;
	return player[sockfd].Parse();
}

void GameSync::Update(int sockfd,PlayerInput input){
	if(!player.count(sockfd))
		return NULL;
	return player[sockfd].Update(input);
}

void GameSync::Retransmission(int sockfd,int beg_fid){
	room[GetRoomId(sockfd)].Retransmission(sockfd,beg_fid);
}

void GameSync::JoinRoom(int sockfd,string name,int room_id){
	if(!room.count(room_id))
		room[room_id]=Room(2);
	player[sockfd]=Player(sockfd,name,room_id);
	name2room[name]=room_id;
	room[room_id].AddPlayer(player[sockfd]);
}

void GameSync::Exit(int sockfd){
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

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
	for(it=room.begin().it!=room.end();it++)
		it->second.Broadcast();
}

