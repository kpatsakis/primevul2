bool CameraService::canConnectUnsafe(int cameraId,
 const String16& clientPackageName,
 const sp<IBinder>& remoteCallback,
                                     sp<BasicClient> &client) {
 String8 clientName8(clientPackageName);
 int callingPid = getCallingPid();

 if (mClient[cameraId] != 0) {
        client = mClient[cameraId].promote();
 if (client != 0) {
 if (remoteCallback == client->getRemote()) {
                LOG1("CameraService::connect X (pid %d) (the same client)",
                     callingPid);
 return true;
 } else {
                ALOGW("CameraService::connect X (pid %d) rejected"
 " (existing client).", callingPid);
 return false;
 }
 }
        mClient[cameraId].clear();
 }

 /*
    mBusy is set to false as the last step of the Client destructor,
    after which it is guaranteed that the Client destructor has finished (
    including any inherited destructors)

    We only need this for a Client subclasses since we don't allow
    multiple Clents to be opened concurrently, but multiple BasicClient
    would be fine
    */
 if (mBusy[cameraId]) {
        ALOGW("CameraService::connect X (pid %d, \"%s\") rejected"
 " (camera %d is still busy).", callingPid,
                clientName8.string(), cameraId);
 return false;
 }

 return true;
}
