#pragma once

#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

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

