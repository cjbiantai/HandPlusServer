#include"server_sync.h"

class GameSync:public ServerSync {
	private:
		char buffer[BUFFER_SIZE];
		char sendbuf[BUFFER_SIZE];
		ConnectionManager connManager;
	public:
		int check(int ret,int sockfd);
		void duplicate(InputList &input);
		void Broadcast();
		void RecvAndHandle(int sockfd);
		void UpdateC2S();
		void Retransmission(int sockfd,int begFid);
};
