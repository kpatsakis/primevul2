static jboolean disableNative(JNIEnv* env, jobject obj) {
    ALOGV("%s:",__FUNCTION__);

    jboolean result = JNI_FALSE;
 if (!sBluetoothInterface) return result;

 int ret = sBluetoothInterface->disable();
 /* Retrun JNI_FALSE only when BTIF explicitly reports
       BT_STATUS_FAIL. It is fine for the BT_STATUS_NOT_READY
       case which indicates that stack had not been enabled.
    */
    result = (ret == BT_STATUS_FAIL) ? JNI_FALSE : JNI_TRUE;
 return result;
}
