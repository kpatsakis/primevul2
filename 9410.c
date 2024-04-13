clock_gettime(clockid_t clock_id, struct timespec *tp)
{
  static int clock_init = 0;
  static clock_serv_t clock;

  mach_timespec_t mts;
  int ret;

  if (! clock_init) {
    clock_init = 1;
    if (host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &clock))
      abort(); /* unlikely */
  }

  if(! tp)
    return -1;

  switch (clock_id) {

  case CLOCK_REALTIME:

    /* query mach for calendar time */
    ret = clock_get_time(clock, &mts);
    if (! ret) {
      tp->tv_sec = mts.tv_sec;
      tp->tv_nsec = mts.tv_nsec;
    }
    break;

  case CLOCK_MONOTONIC:

    /* query mach for monotinic time */
    ret = clock_get_time(clock_port, &mts);
    if (! ret) {
      tp->tv_sec = mts.tv_sec;
      tp->tv_nsec = mts.tv_nsec;
    }
    break;

  default:
    ret = -1;
    break;
  }

  return ret;
}