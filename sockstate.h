struct SockState{
	string name;
	bool online,updated;
	char buffer[SIZE<<1];
	int len,room_id;
	InputList input;
};

class SockstateManager{
	private:
		map<int,SockState> sockstate;
};
