void CameraService::updateStatus(ICameraServiceListener::Status status,
 int32_t cameraId,
 const StatusVector *rejectSourceStates) {
 Mutex::Autolock lock(mStatusMutex);

 ICameraServiceListener::Status oldStatus = mStatusList[cameraId];

    mStatusList[cameraId] = status;

 if (oldStatus != status) {
        ALOGV("%s: Status has changed for camera ID %d from 0x%x to 0x%x",
              __FUNCTION__, cameraId, (uint32_t)oldStatus, (uint32_t)status);

 if (oldStatus == ICameraServiceListener::STATUS_NOT_PRESENT &&
 (status != ICameraServiceListener::STATUS_PRESENT &&
             status != ICameraServiceListener::STATUS_ENUMERATING)) {

            ALOGW("%s: From NOT_PRESENT can only transition into PRESENT"
 " or ENUMERATING", __FUNCTION__);
            mStatusList[cameraId] = oldStatus;
 return;
 }

 if (rejectSourceStates != NULL) {
 const StatusVector &rejectList = *rejectSourceStates;
 StatusVector::const_iterator it = rejectList.begin();

 /**
             * Sometimes we want to conditionally do a transition.
             * For example if a client disconnects, we want to go to PRESENT
             * only if we weren't already in NOT_PRESENT or ENUMERATING.
             */
 for (; it != rejectList.end(); ++it) {
 if (oldStatus == *it) {
                    ALOGV("%s: Rejecting status transition for Camera ID %d, "
 " since the source state was was in one of the bad "
 " states.", __FUNCTION__, cameraId);
                    mStatusList[cameraId] = oldStatus;
 return;
 }
 }
 }

 /**
          * ProClients lose their exclusive lock.
          * - Done before the CameraClient can initialize the HAL device,
          *   since we want to be able to close it before they get to initialize
          */
 if (status == ICameraServiceListener::STATUS_NOT_AVAILABLE) {
 Vector<wp<ProClient> > proClients(mProClientList[cameraId]);
 Vector<wp<ProClient> >::const_iterator it;

 for (it = proClients.begin(); it != proClients.end(); ++it) {
                sp<ProClient> proCl = it->promote();
 if (proCl.get() != NULL) {
                    proCl->onExclusiveLockStolen();
 }
 }
 }

 Vector<sp<ICameraServiceListener> >::const_iterator it;
 for (it = mListenerList.begin(); it != mListenerList.end(); ++it) {
 (*it)->onStatusChanged(status, cameraId);
 }
 }
}
