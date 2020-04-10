#incude"common/all.h"
#include"game_sync.h"

void GameSync::RecvAndHandle(int sockfd){
	gameManager.Recv(sockfd);
	ClientMsg cmsg;
	while(cmsg=gameManager.Parse(sockfd)!=NULL){
		switch(cmsg.type()){
			case 2:
				gameManager.JoinRoom(sockfd,cmsg.name(),cmsg.id());
				break;
			case 3:
				gameManager.Update(sockfd,cmsg.input());
				break;
			case 4:
				gameManager.Retransmission(sockfd,cmsg.id());
				break;
			default:
				return;
		}
	}
}

void GameSync::Broadcast(){
	gameManager.Broadcast();
}

