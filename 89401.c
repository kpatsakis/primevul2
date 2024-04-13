status_t Parcel::writeParcelFileDescriptor(int fd, int commChannel) {
 status_t status;

 if (fd < 0) {
        status = writeInt32(0); // ParcelFileDescriptor is null
 if (status) return status;
 } else {
        status = writeInt32(1); // ParcelFileDescriptor is not null
 if (status) return status;
        status = writeDupFileDescriptor(fd);
 if (status) return status;
 if (commChannel < 0) {
            status = writeInt32(0); // commChannel is null
 if (status) return status;
 } else {
            status = writeInt32(1); // commChannel is not null
 if (status) return status;
            status = writeDupFileDescriptor(commChannel);
 }
 }
 return status;
}
