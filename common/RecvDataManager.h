#pragma once
#include "consts.h"

class RecvDataManager
{
public:
    RecvDataManager();
    ~RecvDataManager(){}
    int EmptySize();
    int GetPackageLength();
    int PushByte(byte);
    byte PopByte();

private:
    byte buff[BUFF_SIZE * 10];
    int head, size;
    int length;
};

