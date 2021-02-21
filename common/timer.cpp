#include "Timer.h"


Timer::Timer()
{
    timeStamp = GetMSecondsNow();
}

long Timer::GetMSecondsNow()
{
    int ret = gettimeofday(&tv, NULL);
    if(ret == -1) 
    {
        printf("GetTimeNow Error: errno = %d, (%s)\n", errno, strerror(errno));
        return -1;
    }
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

bool Timer::WaitForMSeconds(long uSeconds)
{
    if(GetMSecondsNow() - timeStamp < uSeconds) return false;
    timeStamp = GetMSecondsNow();
    return true;
}
