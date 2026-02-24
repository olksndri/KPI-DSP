#include "timeops.h"

double current_time_ms() 
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    double s = ((float)ts.tv_sec) * 1000.0f; 
    double ms = ((float)ts.tv_nsec) / 1000000.0f; 
    return s + ms;
}
