#include "RecvDataManager.h"

RecvDataManager::RecvDataManager() {
    head = 0;
    size = BUFF_SIZE * 10;
}


int RecvDataManager::EmptySize() {
    return size - length;
}

int RecvDataManager::GetPackageLength() {
    if(EmptySize() < HEAD_LENGTH) {
        return -1;
    }
    int ret = 0;
    for(int i = 0; i < HEAD_LENGTH; ++i) {
        ret += buff[(head + i) % size] << ((HEAD_LENGTH -i - 1) * 8);
    }
    return ret;
}
int RecvDataManager::PushByte(byte data) {
    if(EmptySize() == 0) {
        return -1;
    }
    buff[(head + length) % size] = data;
    length += 1;
    return 0;
}

byte RecvDataManager::PopByte() {
    if(length == 0) {
        return -1;
    }
    length -= 1;
    byte ret = buff[head];
    head = (head + 1) % size;
    return ret;
}

