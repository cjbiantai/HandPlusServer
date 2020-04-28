#pragma once
#include "consts.h"
#include "heads.h"

class recvDataManager
{
public:
    recvDataManager();
    ~recvDataManager(){}
    int EmptySize();
    int GetHeaderType();
    int GetPackageLength();
    int PushByte(byte);
    byte PopByte();
    void Log();

private:
    byte buff[BUFF_SIZE * 10];
    int head, size;
    int length;
};

