status_t Parcel::writeDupFileDescriptor(int fd)
{
 int dupFd = dup(fd);
 if (dupFd < 0) {
 return -errno;
 }
 status_t err = writeFileDescriptor(dupFd, true /*takeOwnership*/);
 if (err != OK) {
        close(dupFd);
 }
 return err;
}
