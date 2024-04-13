static int open_flags_to_access_mode(int open_flags) {
 if ((open_flags & O_ACCMODE) == O_RDONLY) {
 return R_OK;
 } else if ((open_flags & O_ACCMODE) == O_WRONLY) {
 return W_OK;
 } else {
 /* Probably O_RDRW, but treat as default to be safe */
 return R_OK | W_OK;
 }
}
