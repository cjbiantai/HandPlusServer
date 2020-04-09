#pragma once
#include "heads.h"

class Timer
{
public:
    Timer();
    bool WaitForMSeconds(long);
    

private:
    struct timeval tv;
    long timeStamp;
    long GetMSecondsNow();
};

