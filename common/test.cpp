#include"../common/all.h"
#include<pthread.h>

#define SIZE 1024
#define N 1000

//#define IP "117.78.9.170"
//#define PORT 17000
#define IP "211.149.255.36"
#define PORT 12345

class Connection{
	public:
        int sockfd;
        struct sockaddr_in saddr;
		char sendbuf[SIZE];
		char buf[SIZE];
        Connection(){
            saddr.sin_family = AF_INET;
            saddr.sin_addr.s_addr = inet_addr(IP);
            saddr.sin_port = htons(PORT);
            sockfd=socket(AF_INET,SOCK_STREAM,0);
            if(connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0){
    	        printf("connect fail\n");
                exit(0);
            }
        }
        ~Connection(){
            close(sockfd);
        }
		bool SendMsg(ClientMsg &cmsg){
			int len=cmsg.ByteSize();
			sendbuf[0]=1;
			sendbuf[4]=(len>>24)&0xff;
			sendbuf[3]=(len>>16)&0xff;
			sendbuf[2]=(len>>8)&0xff;
			sendbuf[1]=len&0xff;
			cmsg.SerializeToArray(sendbuf+HEADER_LEN,len);
			if(send(sockfd,sendbuf,len+HEADER_LEN,0)<=0){
				printf("fd: %d, send fail\n",sockfd);
				return false;
			}
		}

		bool SendRand(){
			for(int i=0;i<SIZE;i++)
				sendbuf[i]=rand();
			if(send(sockfd,sendbuf,SIZE,0)<=0){
				printf("fd: %d, send fail\n",sockfd);
				return false;
			}
		}

		bool RecvMsg(ServerMsg &smsg){
			if(recv(sockfd,buf,SIZE,0)<=0){
				printf("fd: %d, recv fail\n",sockfd);
				return false;
			}
			int len=(buf[4]<<24)+(buf[3]<<16)+(buf[2]<<8)+buf[1];
			if(!smsg.ParseFromArray(buf+5,len)){
				//printf("fd: %d, parse fail\n",sockfd);
				//return false;
			}
			return true;
		}

        bool Login(string account,string password){
            ClientMsg cmsg;
            PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
            cmsg.set_type(LogIn);
            playerinfo->set_account(account);
            playerinfo->set_password(password);
            return SendMsg(cmsg);
        }
        
        bool JoinRoom(int uid,int roomid){
            ClientMsg cmsg;
            PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
            cmsg.set_type(EnterRoom);
            playerinfo->set_uid(uid);
            playerinfo->set_roomid(roomid);
            return SendMsg(cmsg);
        }

        bool Update(){
            ClientMsg cmsg;
            return SendMsg(cmsg);
        }
};

struct Params{
	int l,r,sockfd;
	Connection *conn;
    Params(){}
	Params(int _l,int _r,Connection *_conn):l(_l),r(_r),conn(_conn){}
}params[N];

void* f(void *id){
	Connection *conn=params[(ll)id].conn;
	int sockfd=params[(ll)id].sockfd;
    int l=params[(ll)id].l,r=params[(ll)id].r;
	ServerMsg smsg;
    for(int i=l;i<r;i++){
    	string account="toad_"+to_string(i);
	    conn->Login(account,"toad");
        cout<<account<<endl;
	    if(!conn->RecvMsg(smsg))
            return NULL;
    }
    return NULL;
}

void* g(void *id){
	Connection *conn=params[(ll)id].conn;
	int sockfd=params[(ll)id].sockfd;
    int l=params[(ll)id].l,r=params[(ll)id].r;
	ServerMsg smsg;
    for(int i=l;i<r;i++){
    	string account="toad_"+to_string(i);
	    conn->JoinRoom(i,i);
        cout<<account<<endl;
    }
    return NULL;
}

int main(){ 
    pthread_t th[N];
    Connection conn[N];
    for(ll i=0;i<N;i++){
        params[i]=Params(1000/N*i,1000/N*(i+1),&conn[i]);
        pthread_create(&th[i],NULL,g,(void*)i);
    }
    for(int i=0;i<N;i++)
        pthread_join(th[i],0);
    return 0;
}
