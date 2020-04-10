#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include"game_proto.pb.h"
#include"consts.h"

using namespace std;
using namespace GameProto;
