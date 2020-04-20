#include"../common/all.h"
#include<pthread.h>

#define SIZE 1024

class Connection{
	public:
        int sockfd;
		char sendbuf[SIZE];
		char buf[SIZE];
        Connection(){
            struct sockaddr_in saddr;
            saddr.sin_family = AF_INET;
            saddr.sin_addr.s_addr = inet_addr("117.78.9.170");
            saddr.sin_port = htons(17000);
            int sockfd=socket(AF_INET,SOCK_STREAM,0);
            if(connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0){
    	        printf("connect fail\n");
                exit(0);
            }
        }
        ~Connection(){
            close(sockfd);
        }
		bool SendMsg(ClientMsg cmsg){
			int len=cmsg.ByteSize();
			sendbuf[0]=1;
			sendbuf[4]=(len>>24)&0xff;
			sendbuf[3]=(len>>16)&0xff;
			sendbuf[2]=(len>>8)&0xff;
			sendbuf[1]=len&0xff;
			cmsg.SerializeToArray(sendbuf+HEADER_LEN,len);
			if(send(sockfd,sendbuf,len+HEADER_LEN,0)<0){
				printf("fd: %d, send fail\n",sockfd);
				return false;
			}
		}

		void SendRand(){
			for(int i=0;i<SIZE;i++)
				sendbuf[i]=rand();
			send(sockfd,sendbuf,SIZE,0);
		}

		bool RecvMsg(ServerMsg &smsg){
			if(recv(sockfd,buf,SIZE,0)<=0){
				printf("fd: %d, recv fail\n",sockfd);
				return false;
			}
			int len=(buf[4]<<24)+(buf[3]<<16)+(buf[2]<<8)+buf[1];
			if(!smsg.ParseFromArray(buf+5,len)){
				printf("fd: %d, parse fail\n",sockfd);
				return false;
			}
			return true;
		}
};

struct Params{
	int l,r,sockfd;
	Connection *conn;
    Params(){}
	Params(int _l,int _r,Connection *_conn):l(_l),r(_r),conn(_conn){}
}params[10];


void* f(void *id){
    cout<<(ll)id<<endl;
	Connection *conn=params[(ll)id].conn;
	int sockfd=params[(ll)id].sockfd;
    int l=params[(ll)id].l,r=params[(ll)id].r;
	ClientMsg cmsg;
	ServerMsg smsg;
	cmsg.set_type(LogIn);
	PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
    playerinfo->set_password("toad");
    playerinfo->set_nickname("toad");
    for(int i=l;i<r;i++){
    	string account="toad_"+to_string(i);
	    playerinfo->set_account(account);
	    conn->SendMsg(cmsg);
	    cout<<account<<": ";
	    if(conn->RecvMsg(smsg))
	    	cout<<smsg.type()<<endl;
        else
            return NULL;
    }
    return NULL;
}


int main(){ 
    /*pthread_t th[10];
    Connection conn[10];
    for(ll i=0;i<4;i++){
        params[i]=Params(2500*i,2500*(i+1),&conn[i]);
        pthread_create(&th[i],NULL,f,(void*)i);
    }
    for(int i=0;i<4;i++)
        pthread_join(th[i],0);*/
    Connection conn;
    ClientMsg cmsg;
    ServerMsg smsg;
    cmsg.set_type(LogIn);
	PlayerInfo *playerinfo=cmsg.mutable_playerinfo();
    playerinfo->set_password("toad");
    playerinfo->set_account("toad_1");
    conn.SendMsg(cmsg);
    conn.RecvMsg(smsg);
    cout<<smsg.type()<<endl;
    return 0;
}
