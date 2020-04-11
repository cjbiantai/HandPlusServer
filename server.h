#include"common/all.h"
#include"server_sync.h"

class Server {
	private:
	    int port,listenfd,epfd;
	    ServerSync *sync;
	    struct epoll_event* events;
	    struct timeval start,end;
	    ll tick;
	public:
	    Server(int port,ServerSync *sync);
	    ~Server();
	    bool Init();
	    void WorkOnce();
};

