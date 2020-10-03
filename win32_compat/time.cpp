#include "sys/time.h"
#include <chrono>

extern "C" {
	int gettimeofday(struct timeval* t, struct timezone* tz) {
		static bool setTimeZone = false;
		struct _timeb timebuffer;
		_ftime_s(&timebuffer);
		t->tv_sec = timebuffer.time;
		t->tv_usec = 1000 * timebuffer.millitm;

		if (tz != nullptr) {
			static bool setTimeZone = false;
			if (!setTimeZone) {
				_tzset();
				setTimeZone = true;
			}

			tz->tz_minuteswest = _timezone / 60;
			tz->tz_dsttime = _daylight;
		}
		return 0;
	}


	int clock_gettime(clockid_t type, struct timespec* tp) {
		switch (type) {
			case CLOCK_MONOTONIC:
			{
				// Always-increasing "relatively-absolute" time, most likely unix epoch
				auto monoTime = std::chrono::steady_clock::now().time_since_epoch();
				tp->tv_sec = (time_t)std::chrono::duration_cast<std::chrono::seconds>(monoTime).count();
				tp->tv_nsec = (long)std::chrono::duration_cast<std::chrono::nanoseconds>(monoTime).count();

				return 0;
			}
			case CLOCK_REALTIME:
			{
				// System-wide wall time clock, can be adjusted by leap seconds and such
				auto realTime = std::chrono::system_clock::now().time_since_epoch();
				tp->tv_sec = (time_t)std::chrono::duration_cast<std::chrono::seconds>(realTime).count();
				tp->tv_nsec = (long)std::chrono::duration_cast<std::chrono::nanoseconds>(realTime).count();

				return 0;
			}
			default:
			{
				errno = ENOTSUP;
				return -1;
			}
		}

	}
}