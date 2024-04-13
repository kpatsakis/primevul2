void on_firmware_memory_dump(char *buffer, int buffer_size) {

 JNIHelper helper(mVM);
 /* ALOGD("on_firmware_memory_dump called, vm = %p, obj = %p, env = %p buffer_size = %d"
            , mVM, mCls, env, buffer_size); */

 if (buffer_size > 0) {
 JNIObject<jbyteArray> dump = helper.newByteArray(buffer_size);
        jbyte *bytes = (jbyte *) (buffer);
        helper.setByteArrayRegion(dump, 0, buffer_size, bytes);
        helper.reportEvent(mCls,"onWifiFwMemoryAvailable","([B)V", dump.get());
 }
}
