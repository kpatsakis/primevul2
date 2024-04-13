static bool doCommand(JNIEnv* env, jstring javaCommand,
 char* reply, size_t reply_len) {
 ScopedUtfChars command(env, javaCommand);
 if (command.c_str() == NULL) {
 return false; // ScopedUtfChars already threw on error.
 }

 if (DBG) {
        ALOGD("doCommand: %s", command.c_str());
 }

 --reply_len; // Ensure we have room to add NUL termination.
 if (::wifi_command(command.c_str(), reply, &reply_len) != 0) {
 return false;
 }

 if (reply_len > 0 && reply[reply_len-1] == '\n') {
        reply[reply_len-1] = '\0';
 } else {
        reply[reply_len] = '\0';
 }
 return true;
}
