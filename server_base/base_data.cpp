#include "base_data.h"

baseData::baseData() {
    data = (char*)malloc(sizeof(char) * (BUFF_SIZE + HEAD_LENGTH));
    buff = (char*)malloc(sizeof(char) * (BUFF_SIZE + HEAD_LENGTH));
}

baseData::~baseData() {
    free(data);
    free(buff);
            
}

void baseData::AddHead(int length) {    
    buff[0] = 0;
    buff[1] = length & 0xff;
    buff[2] = (length >> 8) & 0xff;
    buff[3] = (length >> 16) & 0xff;
    buff[4] = (length >> 24) & 0xff;
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

