#pragma once
#include"common/all.h"

class ServerSync{
	public:
		virtual void Broadcast()=0;
		virtual void RecvAndHandle(int)=0;
		virtual void Update()=0;
};
