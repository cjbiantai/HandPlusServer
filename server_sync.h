#pragma once
class ServerSync{
	public:
		virtual void Broadcast()=0;
		virtual void RecvAndHandle(int)=0;
		virtual void Update()=0;
};
