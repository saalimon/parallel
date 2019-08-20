#ifndef CPUCLOCK_C
#define CPUCLOCK_C

#include <sys/times.h>  // Defines the struct tms
#include <unistd.h>
#include <time.h>

// Possibly restricted to Linux:
// return as floating point seconds both the CPU time
// and the wall-clock time.
void getTimes( double *wallClock, double *cpuClock )
{
   static double cvt = 0.0;
   struct tms cpu;
   double wall = times( &cpu );

   if ( cvt == 0.0 )
      cvt = 1.0 / sysconf(_SC_CLK_TCK);
   *wallClock = cvt * wall;
   *cpuClock  = cvt * cpu.tms_utime;
}

// Return ONLY the CPU time.
double cpuClock()
{  double rtnVal, dmy;
   getTimes (&dmy, &rtnVal);
   return rtnVal;
}

#endif