#include "recv_data_manager.h"

recvDataManager::recvDataManager() {
    head = 0;
    length = 0;
    size = BUFF_SIZE * 10;
}


int recvDataManager::EmptySize() {
    return size - length;
}

int recvDataManager::GetHeaderType() {
    if(length < HEAD_LENGTH) {
        return -1;
    }
    if(buff[head] != C2SType && buff[head] != S2SType) return -2;
    return buff[head];
}
int recvDataManager::GetPackageLength() {
    if(length < HEAD_LENGTH) {
        return -1;
    }
    int ret = 0;
    for(int i = 1; i < HEAD_LENGTH; ++i) {
        ret += buff[(head + i) % size] << ((i - 1) * 8);
    }
    return ret;
}
int recvDataManager::PushByte(byte data) {
    if(EmptySize() == 0) {
        return -1;
    }
    buff[(head + length) % size] = data;
    length += 1;
    return 0;
}

byte recvDataManager::PopByte() {
    if(length == 0) {
        return -1;
    }
    length -= 1;
    byte ret = buff[head];
    head = (head + 1) % size;
    return ret;
}

void recvDataManager::Log() {
    for(int i = 0; i < length; ++i) {
        printf("%03d ", buff[(head + i) % size]);
    }
    puts("");
}

