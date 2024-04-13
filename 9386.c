timespec_cmp(struct timespec time1, struct timespec time2)
{
  /* Less than. */
  if (time1.tv_sec < time2.tv_sec)
    return -1;
  /* Greater than. */
  else if (time1.tv_sec > time2.tv_sec)
    return 1;
  /* Less than. */
  else if (time1.tv_nsec < time2.tv_nsec)
    return -1;
  /* Greater than. */
  else if (time1.tv_nsec > time2.tv_nsec)
    return 1;
  /* Equal. */
  else
    return 0;
}