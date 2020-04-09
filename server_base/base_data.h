
#pragma once
#include "../common/consts.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 class baseData
{
public:
    baseData();
    ~baseData();
    void AddHead(int); 
    void ChangeDataAt(char, int);
    void ChangeBuffAt(char, int);
    char* GetDataArray();
    char* GetBuffArray();
    char GetDataCharAt(int);
    char GetBuffCharAt(int);
private:
    char *data, *buff;
};

