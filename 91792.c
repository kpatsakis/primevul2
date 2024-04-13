status_t Parcel::writeParcelable(const Parcelable& parcelable) {
 status_t status = writeInt32(1); // parcelable is not null.
 if (status != OK) {
 return status;
 }
 return parcelable.writeToParcel(this);
}
