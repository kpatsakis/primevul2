void Camera2Client::disconnect() {
    ATRACE_CALL();
 Mutex::Autolock icl(mBinderSerializationLock);

 int callingPid = getCallingPid();
 if (callingPid != mClientPid && callingPid != mServicePid) return;

 if (mDevice == 0) return;

    ALOGV("Camera %d: Shutting down", mCameraId);

 /**
     * disconnect() cannot call any methods that might need to promote a
     * wp<Camera2Client>, since disconnect can be called from the destructor, at
     * which point all such promotions will fail.
     */

    stopPreviewL();

 {
 SharedParameters::Lock l(mParameters);
 if (l.mParameters.state == Parameters::DISCONNECTED) return;
        l.mParameters.state = Parameters::DISCONNECTED;
 }

    mStreamingProcessor->requestExit();
    mFrameProcessor->requestExit();
    mCaptureSequencer->requestExit();
    mJpegProcessor->requestExit();
    mZslProcessorThread->requestExit();
    mCallbackProcessor->requestExit();

    ALOGV("Camera %d: Waiting for threads", mCameraId);

    mStreamingProcessor->join();
    mFrameProcessor->join();
    mCaptureSequencer->join();
    mJpegProcessor->join();
    mZslProcessorThread->join();
    mCallbackProcessor->join();

    ALOGV("Camera %d: Deleting streams", mCameraId);

    mStreamingProcessor->deletePreviewStream();
    mStreamingProcessor->deleteRecordingStream();
    mJpegProcessor->deleteStream();
    mCallbackProcessor->deleteStream();
    mZslProcessor->deleteStream();

    ALOGV("Camera %d: Disconnecting device", mCameraId);

    mDevice->disconnect();

    mDevice.clear();

 CameraService::Client::disconnect();
}
