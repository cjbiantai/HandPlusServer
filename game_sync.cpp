#incude"common/all.h"
#include"game_sync.h"

void GameSync::RecvAndHandle(int sockfd){
	gameManager.Recv(sockfd);
	ClientMsg cmsg;
	while(cmsg=gameManager.Parse(sockfd)!=NULL){
		switch(cmsg.type()){
			case 2:
				gameManager.Update(sockfd,cmsg.input());
				break;
			case 3:
				gameManager.Retransmission(sockfd,cmsg.id());
				break;
			case 4:
				gameManager.JoinRoom(sockfd,cmsg.id());
				break;
			default:
				return;
		}
	}
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
	if(check(SendMsg(sockfd,(*p_frames)[beg_fid]),sockfd)<0)
		return;
	for(int i=begFid;i<p_frames->size();i++)
		if(check(SendMsg(sockfd,(*p_frames)[i]),sockfd)<0)
			return;
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

