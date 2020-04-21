#include"../common/all.h"
#include<pthread.h>

#define SIZE 1024
#define N 1000

class Connection{
	public:
        int sockfd;
        struct sockaddr_in saddr;
		char sendbuf[SIZE];
		char buf[SIZE];
        Connection(){
            saddr.sin_family = AF_INET;
            saddr.sin_addr.s_addr = inet_addr("117.78.9.170");
            saddr.sin_port = htons(17000);
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

        bool Login(string name,string password){
            ClientMsg cmsg;
            PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
            cmsg.set_type(LogIn);
            playerinfo_set_name(name);
            playerinfo.set_password(password);
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
	    if(!conn->RecvMsg(smsg))
            return NULL;
    }
    return NULL;
}

int main(){ 
    pthread_t th[N];
    Connection conn[N];
    for(ll i=0;i<N;i++){
        params[i]=Params(10000/N*i,10000/N*(i+1),&conn[i]);
        pthread_create(&th[i],NULL,f,(void*)i);
    }
    for(int i=0;i<N;i++)
        pthread_join(th[i],0);
    return 0;
}
