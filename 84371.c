void V4L2JpegEncodeAccelerator::EncodedInstanceDmaBuf::FillQuantizationTable(
    int quality,
    const uint8_t* basic_table,
    uint8_t* dst_table) {
  unsigned int temp;

  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality * 2;

  for (size_t i = 0; i < kDctSize; i++) {
    temp = ((unsigned int)basic_table[kZigZag8x8[i]] * quality + 50) / 100;
    /* limit the values to the valid range */
    dst_table[i] = base::ClampToRange(temp, 1u, 255u);
  }
}
