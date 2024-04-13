timer_create(clockid_t clock_id, void *sevp, timer_t *timer_id)
{
  if (clock_id != CLOCK_MONOTONIC)
    return -1;
  if (sevp)
    return -1;

  /* setitimer only supports one timer */
  *timer_id = 0;

  return 0;
}