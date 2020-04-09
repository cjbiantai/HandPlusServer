#pragma once
class ServerSync{
	public:
		virtual void Broadcast(){}
		virtual void RecvAndHandle(int){}
		virtual void Update(){}
};
