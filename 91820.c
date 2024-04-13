static void adapter_properties_callback(bt_status_t status, int num_properties,
 bt_property_t *properties) {
    jobjectArray props;
    jintArray types;
    jbyteArray val;
    jclass mclass;

 if (!checkCallbackThread()) {
       ALOGE("Callback: '%s' is not called on the correct thread", __FUNCTION__);
 return;
 }

    ALOGV("%s: Status is: %d, Properties: %d", __FUNCTION__, status, num_properties);

 if (status != BT_STATUS_SUCCESS) {
        ALOGE("%s: Status %d is incorrect", __FUNCTION__, status);
 return;
 }

    val = (jbyteArray) callbackEnv->NewByteArray(num_properties);
 if (val == NULL) {
        ALOGE("%s: Error allocating byteArray", __FUNCTION__);
 return;
 }

    mclass = callbackEnv->GetObjectClass(val);

 /* (BT) Initialize the jobjectArray and jintArray here itself and send the
     initialized array pointers alone to get_properties */

    props = callbackEnv->NewObjectArray(num_properties, mclass,
                                             NULL);
 if (props == NULL) {
        ALOGE("%s: Error allocating object Array for properties", __FUNCTION__);
 return;
 }

    types = (jintArray)callbackEnv->NewIntArray(num_properties);

 if (types == NULL) {
        ALOGE("%s: Error allocating int Array for values", __FUNCTION__);
 return;
 }
    callbackEnv->DeleteLocalRef(mclass);
    callbackEnv->DeleteLocalRef(val);

 if (get_properties(num_properties, properties, &types, &props) < 0) {
 if (props) callbackEnv->DeleteLocalRef(props);
 if (types) callbackEnv->DeleteLocalRef(types);
 return;
 }

    callbackEnv->CallVoidMethod(sJniCallbacksObj, method_adapterPropertyChangedCallback, types,
                                props);
    checkAndClearExceptionFromCallback(callbackEnv, __FUNCTION__);
    callbackEnv->DeleteLocalRef(props);
    callbackEnv->DeleteLocalRef(types);
 return;

}
