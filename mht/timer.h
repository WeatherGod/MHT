
#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include <time.h>
#define TIMER_ON

static const double clock_tics_per_sec = 1000000.0;
class Timer
{
    clock_t last_time;
    clock_t curn_time;
    double  time_dur;

public:

    void reset_clock()
    {
        last_time = curn_time = clock();
    }

    Timer()
    {
        time_dur=0.0;
        reset_clock();
    }

    void report_cumm_time(char* s)
    {
#ifdef TIMER_ON
        curn_time = clock();
        double tmp = (curn_time-last_time)/clock_tics_per_sec;
        time_dur += tmp;
        printf("%s: %f secs\n",s, time_dur);
#endif
    }

    void report_time(char* s)
    {
#ifdef TIMER_ON
        curn_time = clock();
        double tmp = (curn_time-last_time)/clock_tics_per_sec;
        if (tmp > 0.0)
        {
            printf("%s: %f secs\n",s, tmp);
        }
        last_time = curn_time;
#endif
    }

    double elapsedTime( void )
    {
        curn_time = clock();
        double tmp = (curn_time - last_time) / clock_tics_per_sec;
        last_time = curn_time;
        return tmp;
    }
};

#endif
