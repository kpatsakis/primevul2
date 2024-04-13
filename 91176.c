status_t HevcParameterSets::makeHvcc(uint8_t *hvcc, size_t *hvccSize,
 size_t nalSizeLength) {
 if (hvcc == NULL || hvccSize == NULL
 || (nalSizeLength != 4 && nalSizeLength != 2)) {
 return BAD_VALUE;
 }
 size_t size = 23; // 23 bytes in the header
 size_t numOfArrays = 0;
 const size_t numNalUnits = getNumNalUnits();
 for (size_t i = 0; i < ARRAY_SIZE(kHevcNalUnitTypes); ++i) {
 uint8_t type = kHevcNalUnitTypes[i];
 size_t numNalus = getNumNalUnitsOfType(type);
 if (numNalus == 0) {
 continue;
 }
 ++numOfArrays;
        size += 3;
 for (size_t j = 0; j < numNalUnits; ++j) {
 if (getType(j) != type) {
 continue;
 }
            size += 2 + getSize(j);
 }
 }
 uint8_t generalProfileSpace, generalTierFlag, generalProfileIdc;
 if (!findParam8(kGeneralProfileSpace, &generalProfileSpace)
 || !findParam8(kGeneralTierFlag, &generalTierFlag)
 || !findParam8(kGeneralProfileIdc, &generalProfileIdc)) {
 return ERROR_MALFORMED;
 }
 uint32_t compatibilityFlags;
 uint64_t constraintIdcFlags;
 if (!findParam32(kGeneralProfileCompatibilityFlags, &compatibilityFlags)
 || !findParam64(kGeneralConstraintIndicatorFlags, &constraintIdcFlags)) {
 return ERROR_MALFORMED;
 }
 uint8_t generalLevelIdc;
 if (!findParam8(kGeneralLevelIdc, &generalLevelIdc)) {
 return ERROR_MALFORMED;
 }
 uint8_t chromaFormatIdc, bitDepthLumaMinus8, bitDepthChromaMinus8;
 if (!findParam8(kChromaFormatIdc, &chromaFormatIdc)
 || !findParam8(kBitDepthLumaMinus8, &bitDepthLumaMinus8)
 || !findParam8(kBitDepthChromaMinus8, &bitDepthChromaMinus8)) {
 return ERROR_MALFORMED;
 }
 if (size > *hvccSize) {
 return NO_MEMORY;
 }
 *hvccSize = size;

 uint8_t *header = hvcc;
    header[0] = 1;
    header[1] = (kGeneralProfileSpace << 6) | (kGeneralTierFlag << 5) | kGeneralProfileIdc;
    header[2] = (compatibilityFlags >> 24) & 0xff;
    header[3] = (compatibilityFlags >> 16) & 0xff;
    header[4] = (compatibilityFlags >> 8) & 0xff;
    header[5] = compatibilityFlags & 0xff;
    header[6] = (constraintIdcFlags >> 40) & 0xff;
    header[7] = (constraintIdcFlags >> 32) & 0xff;
    header[8] = (constraintIdcFlags >> 24) & 0xff;
    header[9] = (constraintIdcFlags >> 16) & 0xff;
    header[10] = (constraintIdcFlags >> 8) & 0xff;
    header[11] = constraintIdcFlags & 0xff;
    header[12] = generalLevelIdc;
    header[13] = 0xf0;
    header[14] = 0;
    header[15] = 0xfc;
    header[16] = 0xfc | chromaFormatIdc;
    header[17] = 0xf8 | bitDepthLumaMinus8;
    header[18] = 0xf8 | bitDepthChromaMinus8;
    header[19] = 0;
    header[20] = 0;
    header[21] = nalSizeLength - 1;
    header[22] = numOfArrays;
    header += 23;
 for (size_t i = 0; i < ARRAY_SIZE(kHevcNalUnitTypes); ++i) {
 uint8_t type = kHevcNalUnitTypes[i];
 size_t numNalus = getNumNalUnitsOfType(type);
 if (numNalus == 0) {
 continue;
 }
        header[0] = type;
        header[1] = (numNalus >> 8) & 0xff;
        header[2] = numNalus & 0xff;
        header += 3;
 for (size_t j = 0; j < numNalUnits; ++j) {
 if (getType(j) != type) {
 continue;
 }
            header[0] = (getSize(j) >> 8) & 0xff;
            header[1] = getSize(j) & 0xff;
 if (!write(j, header + 2, size - (header - (uint8_t *)hvcc))) {
 return NO_MEMORY;
 }
            header += (2 + getSize(j));
 }
 }
    CHECK_EQ(header - size, hvcc);

 return OK;
}
