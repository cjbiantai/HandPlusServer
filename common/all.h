#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include"game_proto.pb.h"

#define HEADER_LEN 5
#define BROADCAST_RATE 50
#define BUFFER_SIZE 2048
#define MAX_EVENTS 10000
#define ROOM_MAX 1
//#define DEBUG

using namespace std;
using namespace GameProto;
typedef long long ll;

