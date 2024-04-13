status_t Camera2Client::autoFocus() {
    ATRACE_CALL();
 Mutex::Autolock icl(mBinderSerializationLock);
    ALOGV("%s: Camera %d", __FUNCTION__, mCameraId);
 status_t res;
 if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

 int triggerId;
 bool notifyImmediately = false;
 bool notifySuccess = false;
 {
 SharedParameters::Lock l(mParameters);
 if (l.mParameters.state < Parameters::PREVIEW) {
 return INVALID_OPERATION;
 }

 /**
          * If the camera does not support auto-focus, it is a no-op and
          * onAutoFocus(boolean, Camera) callback will be called immediately
          * with a fake value of success set to true.
          *
          * Similarly, if focus mode is set to INFINITY, there's no reason to
          * bother the HAL.
          */
 if (l.mParameters.focusMode == Parameters::FOCUS_MODE_FIXED ||
                l.mParameters.focusMode == Parameters::FOCUS_MODE_INFINITY) {
            notifyImmediately = true;
            notifySuccess = true;
 }
 /**
         * If we're in CAF mode, and AF has already been locked, just fire back
         * the callback right away; the HAL would not send a notification since
         * no state change would happen on a AF trigger.
         */
 if ( (l.mParameters.focusMode == Parameters::FOCUS_MODE_CONTINUOUS_PICTURE ||
                l.mParameters.focusMode == Parameters::FOCUS_MODE_CONTINUOUS_VIDEO) &&
                l.mParameters.focusState == ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED ) {
            notifyImmediately = true;
            notifySuccess = true;
 }
 /**
         * Send immediate notification back to client
         */
 if (notifyImmediately) {
 SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);
 if (l.mRemoteCallback != 0) {
                l.mRemoteCallback->notifyCallback(CAMERA_MSG_FOCUS,
                        notifySuccess ? 1 : 0, 0);
 }
 return OK;
 }
 /**
         * Handle quirk mode for AF in scene modes
         */
 if (l.mParameters.quirks.triggerAfWithAuto &&
                l.mParameters.sceneMode != ANDROID_CONTROL_SCENE_MODE_UNSUPPORTED &&
                l.mParameters.focusMode != Parameters::FOCUS_MODE_AUTO &&
 !l.mParameters.focusingAreas[0].isEmpty()) {
            ALOGV("%s: Quirk: Switching from focusMode %d to AUTO",
                    __FUNCTION__, l.mParameters.focusMode);
            l.mParameters.shadowFocusMode = l.mParameters.focusMode;
            l.mParameters.focusMode = Parameters::FOCUS_MODE_AUTO;
            updateRequests(l.mParameters);
 }

        l.mParameters.currentAfTriggerId = ++l.mParameters.afTriggerCounter;
        triggerId = l.mParameters.currentAfTriggerId;
 }
    ATRACE_ASYNC_BEGIN(kAutofocusLabel, triggerId);

    syncWithDevice();

    mDevice->triggerAutofocus(triggerId);

 return OK;
}
