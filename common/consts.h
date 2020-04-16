#pragma once

#define LOG_IN 1
#define SIGN_UP 2
#define SIGN_UP_OFFSET 10
#define PLAYER 3
#define RESPONSE 4
#define FRAME_DATA 6
#define CAN_INIT_CHARACTER 7
#define REQUEST_FRAMEDATAS 8
#define PER_FRAME_TIME 50000
#define LENGTH_BASE 100 
#define BUFF_SIZE 10240
#define BACKLOG 100
#define MAX_EVENTS 1024
#define HEAD_LENGTH 5
#define ONE_ROOM 1
#define MAX_FRAMEDATAS 100000
const int HALL_PORT = 17000;
const int SERVER_PORTS[] = {17001, 17002, 17003, 17004, 17005};
const int inf = 1e9+7;

typedef unsigned char byte;
typedef unsigned int uint;
