static void processWakeupCallback(int fd, short flags, void *param) {
 char buff[16];
 int ret;

    RLOGV("processWakeupCallback");

 /* empty our wakeup socket out */
 do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
 } while (ret > 0 || (ret < 0 && errno == EINTR));
}
