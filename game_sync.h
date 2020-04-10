#include"server_sync.h"

class GameSync:public ServerSync {
	private:
		char buffer[BUFFER_SIZE];
		char sendbuf[BUFFER_SIZE];
		GameManager gameManager;
	public:
		void RecvAndHandle(int sockfd);
		void Broadcast();
};
