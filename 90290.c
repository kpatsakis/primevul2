CameraMetadata Camera3Device::RequestThread::getLatestRequest() const {
 Mutex::Autolock al(mLatestRequestMutex);

    ALOGV("RequestThread::%s", __FUNCTION__);

 return mLatestRequest;
}
