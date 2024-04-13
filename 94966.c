WriteEventsToClient(ClientPtr pClient, int count, xEvent *events)
{
#ifdef PANORAMIX
    xEvent eventCopy;
#endif
    xEvent *eventTo, *eventFrom;
    int i, eventlength = sizeof(xEvent);

    if (!pClient || pClient == serverClient || pClient->clientGone)
        return;

    for (i = 0; i < count; i++)
        if ((events[i].u.u.type & 0x7f) != KeymapNotify)
            events[i].u.u.sequenceNumber = pClient->sequence;

    /* Let XKB rewrite the state, as it depends on client preferences. */
    XkbFilterEvents(pClient, count, events);

#ifdef PANORAMIX
    if (!noPanoramiXExtension &&
        (screenInfo.screens[0]->x || screenInfo.screens[0]->y)) {
        switch (events->u.u.type) {
        case MotionNotify:
        case ButtonPress:
        case ButtonRelease:
        case KeyPress:
        case KeyRelease:
        case EnterNotify:
        case LeaveNotify:
            /*
               When multiple clients want the same event DeliverEventsToWindow
               passes the same event structure multiple times so we can't
               modify the one passed to us
             */
            count = 1;          /* should always be 1 */
            memcpy(&eventCopy, events, sizeof(xEvent));
            eventCopy.u.keyButtonPointer.rootX += screenInfo.screens[0]->x;
            eventCopy.u.keyButtonPointer.rootY += screenInfo.screens[0]->y;
            if (eventCopy.u.keyButtonPointer.event ==
                eventCopy.u.keyButtonPointer.root) {
                eventCopy.u.keyButtonPointer.eventX += screenInfo.screens[0]->x;
                eventCopy.u.keyButtonPointer.eventY += screenInfo.screens[0]->y;
            }
            events = &eventCopy;
            break;
        default:
            break;
        }
    }
#endif

    if (EventCallback) {
        EventInfoRec eventinfo;

        eventinfo.client = pClient;
        eventinfo.events = events;
        eventinfo.count = count;
        CallCallbacks(&EventCallback, (void *) &eventinfo);
    }
#ifdef XSERVER_DTRACE
    if (XSERVER_SEND_EVENT_ENABLED()) {
        for (i = 0; i < count; i++) {
            XSERVER_SEND_EVENT(pClient->index, events[i].u.u.type, &events[i]);
        }
    }
#endif
    /* Just a safety check to make sure we only have one GenericEvent, it just
     * makes things easier for me right now. (whot) */
    for (i = 1; i < count; i++) {
        if (events[i].u.u.type == GenericEvent) {
            ErrorF("[dix] TryClientEvents: Only one GenericEvent at a time.\n");
            return;
        }
    }

    if (events->u.u.type == GenericEvent) {
        eventlength += ((xGenericEvent *) events)->length * 4;
    }

    if (pClient->swapped) {
        if (eventlength > swapEventLen) {
            swapEventLen = eventlength;
            swapEvent = realloc(swapEvent, swapEventLen);
            if (!swapEvent) {
                FatalError("WriteEventsToClient: Out of memory.\n");
                return;
            }
        }

        for (i = 0; i < count; i++) {
            eventFrom = &events[i];
            eventTo = swapEvent;

            /* Remember to strip off the leading bit of type in case
               this event was sent with "SendEvent." */
            (*EventSwapVector[eventFrom->u.u.type & 0177])
                (eventFrom, eventTo);

            WriteToClient(pClient, eventlength, eventTo);
        }
    }
    else {
        /* only one GenericEvent, remember? that means either count is 1 and
         * eventlength is arbitrary or eventlength is 32 and count doesn't
         * matter. And we're all set. Woohoo. */
        WriteToClient(pClient, count * eventlength, events);
    }
}
