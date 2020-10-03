#ifndef _TIMES_H
#define _TIMES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1

struct timezone {
	int  tz_minuteswest; /* aka UTC+0 time */
	int  tz_dsttime;     /* daylight saving time */
};

int gettimeofday(struct timeval* t, struct timezone* timezone);

typedef int clockid_t;
#endif

#ifdef __cplusplus
}
#endif

#endif