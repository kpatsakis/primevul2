RIL_register (const RIL_RadioFunctions *callbacks) {
 int ret;
 int flags;

    RLOGI("SIM_COUNT: %d", SIM_COUNT);

 if (callbacks == NULL) {
        RLOGE("RIL_register: RIL_RadioFunctions * null");
 return;
 }
 if (callbacks->version < RIL_VERSION_MIN) {
        RLOGE("RIL_register: version %d is to old, min version is %d",
             callbacks->version, RIL_VERSION_MIN);
 return;
 }

    RLOGE("RIL_register: RIL version %d", callbacks->version);

 if (s_registerCalled > 0) {
        RLOGE("RIL_register has been called more than once. "
 "Subsequent call ignored");
 return;
 }

    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

 /* Initialize socket1 parameters */
    s_ril_param_socket = {
                        RIL_SOCKET_1, /* socket_id */
 -1, /* fdListen */
 -1, /* fdCommand */
                        PHONE_PROCESS, /* processName */
 &s_commands_event, /* commands_event */
 &s_listen_event, /* listen_event */
                        processCommandsCallback, /* processCommandsCallback */
                        NULL, /* p_rs */
                        RIL_TELEPHONY_SOCKET      /* type */
 };

#if (SIM_COUNT >= 2)
    s_ril_param_socket2 = {
                        RIL_SOCKET_2, /* socket_id */
 -1, /* fdListen */
 -1, /* fdCommand */
                        PHONE_PROCESS, /* processName */
 &s_commands_event_socket2, /* commands_event */
 &s_listen_event_socket2, /* listen_event */
                        processCommandsCallback, /* processCommandsCallback */
                        NULL, /* p_rs */
                        RIL_TELEPHONY_SOCKET        /* type */
 };
#endif

#if (SIM_COUNT >= 3)
    s_ril_param_socket3 = {
                        RIL_SOCKET_3, /* socket_id */
 -1, /* fdListen */
 -1, /* fdCommand */
                        PHONE_PROCESS, /* processName */
 &s_commands_event_socket3, /* commands_event */
 &s_listen_event_socket3, /* listen_event */
                        processCommandsCallback, /* processCommandsCallback */
                        NULL, /* p_rs */
                        RIL_TELEPHONY_SOCKET        /* type */
 };
#endif

#if (SIM_COUNT >= 4)
    s_ril_param_socket4 = {
                        RIL_SOCKET_4, /* socket_id */
 -1, /* fdListen */
 -1, /* fdCommand */
                        PHONE_PROCESS, /* processName */
 &s_commands_event_socket4, /* commands_event */
 &s_listen_event_socket4, /* listen_event */
                        processCommandsCallback, /* processCommandsCallback */
                        NULL, /* p_rs */
                        RIL_TELEPHONY_SOCKET        /* type */
 };
#endif


    s_registerCalled = 1;

    RLOGI("s_registerCalled flag set, %d", s_started);

 for (int i = 0; i < (int)NUM_ELEMS(s_commands); i++) {
        assert(i == s_commands[i].requestNumber);
 }

 for (int i = 0; i < (int)NUM_ELEMS(s_unsolResponses); i++) {
        assert(i + RIL_UNSOL_RESPONSE_BASE
 == s_unsolResponses[i].requestNumber);
 }


 if (s_started == 0) {
        RIL_startEventLoop();
 }

    startListen(RIL_SOCKET_1, &s_ril_param_socket);

#if (SIM_COUNT >= 2)
    startListen(RIL_SOCKET_2, &s_ril_param_socket2);
#endif /* (SIM_COUNT == 2) */

#if (SIM_COUNT >= 3)
    startListen(RIL_SOCKET_3, &s_ril_param_socket3);
#endif /* (SIM_COUNT == 3) */

#if (SIM_COUNT >= 4)
    startListen(RIL_SOCKET_4, &s_ril_param_socket4);
#endif /* (SIM_COUNT == 4) */


#if 1

 char *inst = NULL;
 if (strlen(RIL_getRilSocketName()) >= strlen(SOCKET_NAME_RIL)) {
        inst = RIL_getRilSocketName() + strlen(SOCKET_NAME_RIL);
 }

 char rildebug[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_DEBUG;
 if (inst != NULL) {
        strlcat(rildebug, inst, MAX_DEBUG_SOCKET_NAME_LENGTH);
 }

    s_fdDebug = android_get_control_socket(rildebug);
 if (s_fdDebug < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
        exit(-1);
 }

    ret = listen(s_fdDebug, 4);

 if (ret < 0) {
        RLOGE("Failed to listen on ril debug socket '%d': %s",
             s_fdDebug, strerror(errno));
        exit(-1);
 }

    ril_event_set (&s_debug_event, s_fdDebug, true,
                debugCallback, NULL);

    rilEventAddWakeup (&s_debug_event);
#endif

}
