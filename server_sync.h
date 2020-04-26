#pragma once
#include"common/all.h"

class ServerSync{
	public:
		virtual void Broadcast()=0;
		virtual void RecvAndHandle(int)=0;
		virtual void Exit(int sockfd)=0;
        virtual void Print(){};
};
