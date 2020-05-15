#pragma once
#include"common/all.h"
#include"socket_error.h"

class SocketConnect{
    private:
        int len;
        unsigned char buffer[BUFFER_SIZE];
        static unsigned char sendbuf[BUFFER_SIZE];
    public:
        int sockfd;
        SocketConnect();
        SocketConnect(int sockfd);
        void Init(int sockfd);
        int Recv();
        int SendMsg(google::protobuf::Message *msg);
        int Parse(google::protobuf::Message *msg);
}; 
