#include "common/all.h"
#include "common/consts.h"
#include "base_data.h"

class Server {
	public:
	    Server(int,DataHandler);
	    ~Server();
	    void WorkOnce();
	
	protected:
	    int port, server_fd;
	    ServerSync sync;
	    struct epoll_event* events;
	    struct epoll_event ev;
	    int epoll_fd;
	    baseData bData;
	private:
	    struct sockaddr_in server_addr;
	    struct timeval start,end;
	    ll tick;
	    void Init();
};

