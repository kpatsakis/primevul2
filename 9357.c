timer_getoverrun(timer_t timer_id)
{
  /* since we don't know if there have been signals that weren't delivered,
     assume none */
  return 0;
}