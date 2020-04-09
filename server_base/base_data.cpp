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
    for(int i = 0; i < HEAD_LENGTH; ++i) {
        buff[i] = (byte)(length >> ((HEAD_LENGTH - i -1)*8));
    }
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

