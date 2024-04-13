clock_getres(clockid_t clock_id, struct timespec *res)
{
  if (! res)
    return -1;

  /* hardcode ms resolution */
  res->tv_sec = 0;
  res->tv_nsec = 1000;

  return 0;
}