internalRequestTimedCallback (RIL_TimedCallback callback, void *param,
 const struct timeval *relativeTime)
{
 struct timeval myRelativeTime;
 UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) calloc(1, sizeof(UserCallbackInfo));
 if (p_info == NULL) {
        RLOGE("Memory allocation failed in internalRequestTimedCallback");
 return p_info;

 }

    p_info->p_callback = callback;
    p_info->userParam = param;

 if (relativeTime == NULL) {
 /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
 } else {
 /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
 }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
 return p_info;
}
