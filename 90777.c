status_t ProCamera2Client::exclusiveLock() {
    ATRACE_CALL();
    ALOGV("%s", __FUNCTION__);

 Mutex::Autolock icl(mBinderSerializationLock);
 SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);

 if (!mDevice.get()) return PERMISSION_DENIED;

 /**
     * TODO: this should asynchronously 'wait' until the lock becomes available
     * if another client already has an exclusive lock.
     *
     * once we have proper sharing support this will need to do
     * more than just return immediately
     */
 if (!mExclusiveLock) {
        mExclusiveLock = true;

 if (mRemoteCallback != NULL) {
            mRemoteCallback->onLockStatusChanged(IProCameraCallbacks::LOCK_ACQUIRED);
 }

        ALOGV("%s: exclusive lock acquired", __FUNCTION__);

 return OK;
 }

    ALOGW("%s: exclusive lock already exists - recursive locking is not allowed"
 , __FUNCTION__);
 return ALREADY_EXISTS;
}
