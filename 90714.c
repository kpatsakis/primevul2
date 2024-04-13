status_t Camera2Client::updateProcessorStream(sp<ProcessorT> processor,
 Parameters params) {
 status_t res;

 ProcessorT *processorPtr = processor.get();
    res = (processorPtr->*updateStreamF)(params);

 /**
     * Can't update the stream if it's busy?
     *
     * Then we need to stop the device (by temporarily clearing the request
     * queue) and then try again. Resume streaming once we're done.
     */
 if (res == -EBUSY) {
        ALOGV("%s: Camera %d: Pausing to update stream", __FUNCTION__,
                mCameraId);
        res = mStreamingProcessor->togglePauseStream(/*pause*/true);
 if (res != OK) {
            ALOGE("%s: Camera %d: Can't pause streaming: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
 }

        res = mDevice->waitUntilDrained();
 if (res != OK) {
            ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
 }

        res = (processorPtr->*updateStreamF)(params);
 if (res != OK) {
            ALOGE("%s: Camera %d: Failed to update processing stream "
 " despite having halted streaming first: %s (%d)",
                  __FUNCTION__, mCameraId, strerror(-res), res);
 }

        res = mStreamingProcessor->togglePauseStream(/*pause*/false);
 if (res != OK) {
            ALOGE("%s: Camera %d: Can't unpause streaming: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
 }
 }

 return res;
}
