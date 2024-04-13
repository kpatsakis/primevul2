status_t CameraDeviceClient::getCameraInfo(/*out*/CameraMetadata* info)
{
    ATRACE_CALL();
    ALOGV("%s", __FUNCTION__);

 status_t res = OK;

 if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

 Mutex::Autolock icl(mBinderSerializationLock);

 if (!mDevice.get()) return DEAD_OBJECT;

 if (info != NULL) {
 *info = mDevice->info(); // static camera metadata
 }

 return res;
}
