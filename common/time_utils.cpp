#include "./time_utils.h"
#include <sys/time.h>  
#include <stddef.h>

namespace Common
{
    long GetCurrentTimeMs()    
    {
        struct timeval tv;    
        gettimeofday(&tv, NULL);    
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;    
    }    
}
