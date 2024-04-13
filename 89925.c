static jint android_net_wifi_getSupportedFeatures(JNIEnv *env, jclass cls, jint iface) {

 JNIHelper helper(env);
    wifi_interface_handle handle = getIfaceHandle(helper, cls, iface);
    feature_set set = 0;

    wifi_error result = WIFI_SUCCESS;
 /*
    set = WIFI_FEATURE_INFRA
        | WIFI_FEATURE_INFRA_5G
        | WIFI_FEATURE_HOTSPOT
        | WIFI_FEATURE_P2P
        | WIFI_FEATURE_SOFT_AP
        | WIFI_FEATURE_GSCAN
        | WIFI_FEATURE_PNO
        | WIFI_FEATURE_TDLS
        | WIFI_FEATURE_EPR;
    */

    result = hal_fn.wifi_get_supported_feature_set(handle, &set);
 if (result == WIFI_SUCCESS) {
 return set;
 } else {
        ALOGE("wifi_get_supported_feature_set returned error = 0x%x", result);
 return 0;
 }
}
