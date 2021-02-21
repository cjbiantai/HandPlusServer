#include "base_data.h"

baseData::baseData() {
    data = (char*)malloc(sizeof(char) * (BUFF_SIZE + HEAD_LENGTH));
    buff = (char*)malloc(sizeof(char) * (BUFF_SIZE + HEAD_LENGTH));
}

baseData::~baseData() {
    free(data);
    free(buff);
}

void baseData::AddHead(int symbol, int length, int lengthBase) {
    buff[0] = symbol;
    buff[1] = length / lengthBase + 1;
    buff[2] = length % lengthBase + 1;
}
void baseData::ChangeDataAt(char c, int idx) {
    data[idx] = c;
}
void baseData::ChangeBuffAt(char c, int idx) {
    buff[idx] = c;
}

char* baseData::GetDataArray() {
    return data;
}

char* baseData::GetBuffArray() {
    return buff;
}

char baseData::GetDataCharAt(int idx) {
    return data[idx];
}

char baseData::GetBuffCharAt(int idx) {
    return buff[idx];
}

