#pragma once
class ServerSync{
	public:
		ServerSync(){};
		virtual void Broadcast(){};
		virtual void RecvAndHandle(int){};
};
