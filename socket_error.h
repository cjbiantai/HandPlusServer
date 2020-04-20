#pragma once
#include"common/all.h"
#include"server_sync.h"

class SocketError{
	public:
		static int epfd;
		static ServerSync *sync;
		static int Check(int ret,int sockfd);
};
