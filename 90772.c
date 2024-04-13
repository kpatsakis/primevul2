status_t ProCamera2Client::createDefaultRequest(int templateId,
 /*out*/
                              camera_metadata** request)
{
    ATRACE_CALL();
    ALOGV("%s (templateId = 0x%x)", __FUNCTION__, templateId);

 if (request) {
 *request = NULL;
 }

 status_t res;
 if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

 Mutex::Autolock icl(mBinderSerializationLock);

 if (!mDevice.get()) return DEAD_OBJECT;

 CameraMetadata metadata;
 if ( (res = mDevice->createDefaultRequest(templateId, &metadata) ) == OK) {
 *request = metadata.release();
 }

 return res;
}
