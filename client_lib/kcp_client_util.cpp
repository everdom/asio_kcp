#include <time.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <thread>
#include <chrono>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "kcp_client_util.h"


namespace asio_kcp {

void millisecond_sleep(size_t n_millisecond)
{
    //struct timespec sleepTime;
    //struct timespec time_left_to_sleep;
    //sleepTime.tv_sec = n_millisecond / 1000;
    //sleepTime.tv_nsec = (n_millisecond % 1000) * 1000 * 1000;
    //while( (sleepTime.tv_sec + sleepTime.tv_nsec) > 0 )
    //{
    //    time_left_to_sleep.tv_sec = 0;
    //    time_left_to_sleep.tv_nsec = 0;
    //    int ret = nanosleep(&sleepTime, &time_left_to_sleep);
    //    if (ret < 0)
    //        std::cerr << "nanosleep error with errno: " << errno << " " << strerror(errno) << std::endl;

    //    sleepTime.tv_sec = time_left_to_sleep.tv_sec;
    //    sleepTime.tv_nsec = time_left_to_sleep.tv_nsec;
    //}
    std::this_thread::sleep_for(std::chrono::milliseconds(n_millisecond));

}


/* get system time */
void itimeofday(long *sec, long *usec)
{

#if defined(WIN32) || defined(WIN64)
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    if (sec) *sec = clock;
    if (usec) *usec = wtm.wMilliseconds * 1000;
#else
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
#endif

}

/* get clock in millisecond 64 */
uint64_t iclock64(void)
{
    long s, u;
    uint64_t value;
    itimeofday(&s, &u);
    value = ((uint64_t)s) * 1000 + (u / 1000);
    return value;
}


uint32_t iclock()
{
    return (uint32_t)(iclock64() & 0xfffffffful);
}

} // end of asio_kcp
