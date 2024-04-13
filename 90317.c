status_t Camera3Device::RequestThread::queueTriggerLocked(
 RequestTrigger trigger) {

 uint32_t tag = trigger.metadataTag;
 ssize_t index = mTriggerMap.indexOfKey(tag);

 switch (trigger.getTagType()) {
 case TYPE_BYTE:
 case TYPE_INT32:
 break;
 default:
            ALOGE("%s: Type not supported: 0x%x", __FUNCTION__,
                    trigger.getTagType());
 return INVALID_OPERATION;
 }

 /**
     * Collect only the latest trigger, since we only have 1 field
     * in the request settings per trigger tag, and can't send more than 1
     * trigger per request.
     */
 if (index != NAME_NOT_FOUND) {
        mTriggerMap.editValueAt(index) = trigger;
 } else {
        mTriggerMap.add(tag, trigger);
 }

 return OK;
}
