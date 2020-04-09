struct Room{
	int max,state;
	vector<int> sockfd;
	vector<ServerMsg> frames;
};

struct SockState{
	string name;
	bool online,updated;
	char buffer[SIZE<<1];
	int len,roomId;
	InputList input;
};

void SendMsg(int sockfd,ServerMsg smsg);
void SendToRoom(int roomId,ServerMsg smsg);

class ConnectionManager{
	private:
		map<int,SockState> sockstate;
		map<string,int> name2room;
		map<int,Room> room;
	public:
		void JoinRoom(int sockfd,int roomId);
		void Exit(int sockfd);
		void InitRoom(int roomId);
		int GetRoomId(int sockfd);
		void AddConnection(string name);
		bool isOnline(int sockfd);
		int GetRoomId(int sockfd);
		friend GameSync;
};
