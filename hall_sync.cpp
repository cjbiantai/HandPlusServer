#include"hall_sync.h"

HallSync::HallSync(int sockfd){
    socketConn.Init(sockfd);
}

int HallSync::SendMsg(S2SMsg *smsg){
    LOG(2,"HallSync::SendMsg");
    return socketConn.SendMsg(smsg);
}

int HallSync::Recv(){
    LOG(2,"HallSync::Recv");
    return socketConn.Recv();
}

int HallSync::Parse(S2SMsg *msg){
    return socketConn.Parse(msg);
}

