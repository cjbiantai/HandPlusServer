#incude"../common/all.h"
#include"game_sync.h"

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
	for(it=room.begin();it!=room.end();it++){
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
		SendToAll(it->first,frame);
	}
}

