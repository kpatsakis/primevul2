status_t Camera3Device::capture(CameraMetadata &request, int64_t* /*lastFrameNumber*/) {
    ATRACE_CALL();

 List<const CameraMetadata> requests;
    requests.push_back(request);
 return captureList(requests, /*lastFrameNumber*/NULL);
}
