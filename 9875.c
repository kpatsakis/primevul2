MakeSampleAesVideoDescriptor(AP4_DataBuffer& descriptor)
{
    descriptor.SetDataSize(6);
    AP4_UI08* payload = descriptor.UseData();
    payload[0] = AP4_MPEG2_PRIVATE_DATA_INDICATOR_DESCRIPTOR_TAG;
    payload[1] = 4;
    payload[2] = 'z';
    payload[3] = 'a';
    payload[4] = 'v';
    payload[5] = 'c';

    return AP4_SUCCESS;
}