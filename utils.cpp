#include <limits>


#ifdef linux
#include <sys/time.h>
unsigned int getMillis()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return currentTime.tv_sec*1000+currentTime.tv_usec/1000;
}

#else
#include <windows.h>
unsigned int getMillis()
{
    return static_cast<unsigned int>(GetTickCount());
}
#endif
unsigned int getTimedif(unsigned int pTimestampOld, unsigned int pTimestampNew)
{
    if(pTimestampOld <= pTimestampNew)
    {
        return pTimestampNew - pTimestampOld;
    }
    else
    {
        #ifdef linux
        return pTimestampNew + (__UINT32_MAX__ - pTimestampOld);
        #else
        return pTimestampNew + (MAXUINT32 - pTimestampOld);
        #endif
    }
}