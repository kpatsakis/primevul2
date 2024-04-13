size_t V4L2JpegEncodeAccelerator::EncodedInstance::FinalizeJpegImage(
    uint8_t* dst_ptr,
    const JpegBufferRecord& output_buffer,
    size_t buffer_size,
    std::unique_ptr<UnalignedSharedMemory> exif_shm) {
  DCHECK(parent_->encoder_task_runner_->BelongsToCurrentThread());
  size_t idx;

  dst_ptr[0] = 0xFF;
  dst_ptr[1] = JPEG_SOI;
  idx = 2;

  if (exif_shm) {
    uint8_t* exif_buffer = static_cast<uint8_t*>(exif_shm->memory());
    size_t exif_buffer_size = exif_shm->size();
    uint16_t exif_segment_size = static_cast<uint16_t>(exif_buffer_size + 2);
    const uint8_t kAppSegment[] = {
        0xFF, JPEG_APP1, static_cast<uint8_t>(exif_segment_size / 256),
        static_cast<uint8_t>(exif_segment_size % 256)};
    memcpy(dst_ptr + idx, kAppSegment, sizeof(kAppSegment));
    idx += sizeof(kAppSegment);
    memcpy(dst_ptr + idx, exif_buffer, exif_buffer_size);
    idx += exif_buffer_size;
  } else {
    static const uint8_t kAppSegment[] = {
        0xFF, JPEG_APP0, 0x00,
        0x10,  // Segment length:16 (2-byte).
        0x4A,  // J
        0x46,  // F
        0x49,  // I
        0x46,  // F
        0x00,  // 0
        0x01,  // Major version.
        0x01,  // Minor version.
        0x01,  // Density units 0:no units, 1:pixels per inch,
        0x00,
        0x48,  // X density (2-byte).
        0x00,
        0x48,  // Y density (2-byte).
        0x00,  // Thumbnail width.
        0x00   // Thumbnail height.
    };
    memcpy(dst_ptr + idx, kAppSegment, sizeof(kAppSegment));
    idx += sizeof(kAppSegment);
  }

  switch (output_buffer_pixelformat_) {
    case V4L2_PIX_FMT_JPEG_RAW:
      memcpy(dst_ptr + idx, jpeg_markers_.data(), jpeg_markers_.size());
      idx += jpeg_markers_.size();
      memcpy(dst_ptr + idx, output_buffer.address[0], buffer_size);
      idx += buffer_size;
      if (dst_ptr[idx - 2] != 0xFF && dst_ptr[idx - 1] != JPEG_EOI) {
        dst_ptr[idx] = 0xFF;
        dst_ptr[idx + 1] = JPEG_EOI;
        idx += 2;
      }
      break;

    default:
      NOTREACHED() << "Unsupported output pixel format";
  }

  return idx;
}
