static int sgi_clock_get(clockid_t clockid, struct timespec *tp)
{
	u64 nsec;
 
 	nsec = rtc_time() * sgi_clock_period
 			+ sgi_clock_offset.tv_nsec;
	*tp = ns_to_timespec(nsec);
	tp->tv_sec += sgi_clock_offset.tv_sec;
 	return 0;
 };
