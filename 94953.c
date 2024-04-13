AllowSome(ClientPtr client, TimeStamp time, DeviceIntPtr thisDev, int newState)
{
    Bool thisGrabbed, otherGrabbed, othersFrozen, thisSynced;
    TimeStamp grabTime;
    DeviceIntPtr dev;
    GrabInfoPtr devgrabinfo, grabinfo = &thisDev->deviceGrab;

    thisGrabbed = grabinfo->grab && SameClient(grabinfo->grab, client);
    thisSynced = FALSE;
    otherGrabbed = FALSE;
    othersFrozen = FALSE;
    grabTime = grabinfo->grabTime;
    for (dev = inputInfo.devices; dev; dev = dev->next) {
        devgrabinfo = &dev->deviceGrab;

        if (dev == thisDev)
            continue;
        if (devgrabinfo->grab && SameClient(devgrabinfo->grab, client)) {
            if (!(thisGrabbed || otherGrabbed) ||
                (CompareTimeStamps(devgrabinfo->grabTime, grabTime) == LATER))
                grabTime = devgrabinfo->grabTime;
            otherGrabbed = TRUE;
            if (grabinfo->sync.other == devgrabinfo->grab)
                thisSynced = TRUE;
            if (devgrabinfo->sync.state >= FROZEN)
                othersFrozen = TRUE;
        }
    }
    if (!((thisGrabbed && grabinfo->sync.state >= FROZEN) || thisSynced))
        return;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
        (CompareTimeStamps(time, grabTime) == EARLIER))
        return;
    switch (newState) {
    case THAWED:               /* Async */
        if (thisGrabbed)
            grabinfo->sync.state = THAWED;
        if (thisSynced)
            grabinfo->sync.other = NullGrab;
        ComputeFreezes();
        break;
    case FREEZE_NEXT_EVENT:    /* Sync */
        if (thisGrabbed) {
            grabinfo->sync.state = FREEZE_NEXT_EVENT;
            if (thisSynced)
                grabinfo->sync.other = NullGrab;
            ComputeFreezes();
        }
        break;
    case THAWED_BOTH:          /* AsyncBoth */
        if (othersFrozen) {
            for (dev = inputInfo.devices; dev; dev = dev->next) {
                devgrabinfo = &dev->deviceGrab;
                if (devgrabinfo->grab && SameClient(devgrabinfo->grab, client))
                    devgrabinfo->sync.state = THAWED;
                if (devgrabinfo->sync.other &&
                    SameClient(devgrabinfo->sync.other, client))
                    devgrabinfo->sync.other = NullGrab;
            }
            ComputeFreezes();
        }
        break;
    case FREEZE_BOTH_NEXT_EVENT:       /* SyncBoth */
        if (othersFrozen) {
            for (dev = inputInfo.devices; dev; dev = dev->next) {
                devgrabinfo = &dev->deviceGrab;
                if (devgrabinfo->grab && SameClient(devgrabinfo->grab, client))
                    devgrabinfo->sync.state = FREEZE_BOTH_NEXT_EVENT;
                if (devgrabinfo->sync.other
                    && SameClient(devgrabinfo->sync.other, client))
                    devgrabinfo->sync.other = NullGrab;
            }
            ComputeFreezes();
        }
        break;
    case NOT_GRABBED:          /* Replay */
        if (thisGrabbed && grabinfo->sync.state == FROZEN_WITH_EVENT) {
            if (thisSynced)
                grabinfo->sync.other = NullGrab;
            syncEvents.replayDev = thisDev;
            syncEvents.replayWin = grabinfo->grab->window;
            (*grabinfo->DeactivateGrab) (thisDev);
            syncEvents.replayDev = (DeviceIntPtr) NULL;
        }
        break;
    case THAW_OTHERS:          /* AsyncOthers */
        if (othersFrozen) {
            for (dev = inputInfo.devices; dev; dev = dev->next) {
                if (dev == thisDev)
                    continue;
                devgrabinfo = &dev->deviceGrab;
                if (devgrabinfo->grab && SameClient(devgrabinfo->grab, client))
                    devgrabinfo->sync.state = THAWED;
                if (devgrabinfo->sync.other
                    && SameClient(devgrabinfo->sync.other, client))
                    devgrabinfo->sync.other = NullGrab;
            }
            ComputeFreezes();
        }
        break;
    }

    /* We've unfrozen the grab. If the grab was a touch grab, we're now the
     * owner and expected to accept/reject it. Reject == ReplayPointer which
     * we've handled in ComputeFreezes() (during DeactivateGrab) above,
     * anything else is accept.
     */
    if (newState != NOT_GRABBED /* Replay */ &&
        IsTouchEvent((InternalEvent*)grabinfo->sync.event)) {
        TouchAcceptAndEnd(thisDev, grabinfo->sync.event->touchid);
    }
}
