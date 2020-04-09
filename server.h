#include "common/all.h"
#include "common/consts.h"
#include "base_data.h"

class Server {
	private:
	    int port,listenfd,epfd;
	    ServerSync sync;
	    struct epoll_event* events;
	    struct timeval start,end;
	    ll tick;
	public:
	    Server(int,DataHandler);
	    ~Server();
	    bool Init();
	    void WorkOnce();
};

