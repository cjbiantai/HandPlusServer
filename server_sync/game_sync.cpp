#incude"../common/all.h"
#include"game_sync.h"

int GameSync::check(int ret,int sockfd){
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
		connManager.Exit(sockfd);
		return 0;
	}
}

void GameSync::RecvAndHandle(int sockfd){
	int ret;
	if(check(ret=recv(sockfd,buffer,SIZE,MSG_DONTWAIT),sockfd)<=0)
		return;
		
	ServerMsg smsg;
	ClientMsg cmsg;
	int &p_len=sockstate[sockfd].len;
	char *p_buf=sockstate[sockfd].buffer;
	for(int i=0;i<len;i++)
		p_buf[i+p_len]=buffer[i];
	p_buf[p_len+=len]=0;
	if(p_len<HEADER_LEN)
		return;
	
	int msg_len,ret,i;
	for(i=0;i+HEADER_LEN<=p_len;i+=msg_len+HEADER_LEN){
		msg_len=(p_buf[i+1]<<24)+(p_buf[i+2]<<16)+(p_buf[i+3]<<8)+p_buf[i+4];
		if(msg_len+HEADER_LEN>SIZE)
			return;
		if(p_len<i+msg_len+HEADER_LEN)
			break;
		ret=cmsg.ParseFromArray(p_buf+i+HEADER_LEN,msg_len);
		if(!ret){
			printf("parse error\n");
			continue;
		}
		//cout<<cmsg.type()<<" "<<cmsg.name()<<" "<<cmsg.password()<<endl;
		
		switch(cmsg.type()){
			case 2:
				UpdateC2S(cmsg,sockfd);
				break;
			case 3:
				Retransmission(sockfd,cmsg.id());
				break;
			case 4:
				connManager.JoinRoom(sockfd,cmsg.id());
				break;
			default:
				return;
		}
	}
	memcpy(p_buf,p_buf+i,p_len-=i);
    return;
}

void GameSync::UpdateC2S(const ClientMsg &cmsg,int sockfd){
	SockState &p_state=sockstate[sockfd];
	if(!connManager.isOnline(sockfd)||!connManager.GetRoomId(sockfd))
		return;
	string input=cmsg.input();
	string *str=p_state.input.add_list();
	*str=input;
	sockstate[sockfd].updated=true;
}

void GameSync::Retransmission(int sockfd,int begFid){
	vector<ServerMsg> *p_frames=&connManager.room[connManager.GetRoomId(sockfd)].frames;
	SendMsg(sockfd,(*p_frames)[beg_fid]);
	for(int i=begFid;i<p_frames->size();i++)
		SendMsg(sockfd,(*p_frames)[i]);
}

void GameSync::duplicate(InputList &input){
	int sz=input.list_size();
	if(sz==0)
		return;
	string tmp=input.list(sz-1);
	input.clear_list();
	string *list=input.add_list();
	*list=tmp;
}

void GameSync::Broadcast(){
	map<int,Room>::iterator it;
	Room *p_room;
	for(it=connManager.room.begin();it!=connManager.room.end();it++){
		p_room=&it->second;
		if(!p_room->state)
			continue;
		ServerMsg frame;
		frame.set_code(2);
		frame.set_fid(p_room->frames.size());
		InputList *p_inputs;
		SockState *p_state;
		for(int i=0;i<p_room->sockfd.size();i++){
			p_state=&sockstate[p_room->sockfd[i]];
			if(!p_state->updated)
				duplicate(p_state->input);
			p_state->updated=false;
			p_inputs=frame.add_inputs();
			*p_inputs=p_state->input;
		}
		p_room->frames.push_back(frame);
		SendToRoom(it->first,frame);
	}
}

