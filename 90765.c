status_t CameraDeviceClient::initialize(camera_module_t *module)
{
    ATRACE_CALL();
 status_t res;

    res = Camera2ClientBase::initialize(module);
 if (res != OK) {
 return res;
 }

 String8 threadName;
    mFrameProcessor = new FrameProcessorBase(mDevice);
    threadName = String8::format("CDU-%d-FrameProc", mCameraId);
    mFrameProcessor->run(threadName.string());

    mFrameProcessor->registerListener(FRAME_PROCESSOR_LISTENER_MIN_ID,
                                      FRAME_PROCESSOR_LISTENER_MAX_ID,
 /*listener*/this,
 /*quirkSendPartials*/true);

 return OK;
}
