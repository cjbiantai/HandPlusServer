#include"common/all.h"
#include"socket_error.h"
#include"server_sync.h"

class Server {
	private:
	    int port,listenfd,hallfd,epfd;
	    ServerSync *sync;
	    struct epoll_event* events;
	    struct timeval start,end;
	    ll tick;
	public:
	    Server(int port,ServerSync *sync);
	    ~Server();
	    void WorkOnce();
};

