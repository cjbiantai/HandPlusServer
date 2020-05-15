#pragma once
#include"common/all.h"
#include"socket_connect.h"

class HallSync{
    public:
        SocketConnect socketConn;
        HallSync(int sockfd);
        int SendMsg(S2SMsg *msg);
        int Recv();
        int Parse(S2SMsg *msg);
        void Sync(int sockfd);
};
