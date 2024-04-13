WriteAdtsHeader(AP4_ByteStream& output,
                unsigned int    frame_size,
                unsigned int    sampling_frequency_index,
                unsigned int    channel_configuration)
{
	unsigned char bits[7];

	bits[0] = 0xFF;
	bits[1] = 0xF1; // 0xF9 (MPEG2)
    bits[2] = 0x40 | (sampling_frequency_index << 2) | (channel_configuration >> 2);
    bits[3] = ((channel_configuration&0x3)<<6) | ((frame_size+7) >> 11);
    bits[4] = ((frame_size+7) >> 3)&0xFF;
	bits[5] = (((frame_size+7) << 5)&0xFF) | 0x1F;
	bits[6] = 0xFC;

	return output.Write(bits, 7);

	/*
        0:  syncword 12 always: '111111111111' 
        12: ID 1 0: MPEG-4, 1: MPEG-2 
        13: layer 2 always: '00' 
        15: protection_absent 1  
        16: profile 2  
        18: sampling_frequency_index 4  
        22: private_bit 1  
        23: channel_configuration 3  
        26: original/copy 1  
        27: home 1  
        28: emphasis 2 only if ID == 0 

        ADTS Variable header: these can change from frame to frame 
        28: copyright_identification_bit 1  
        29: copyright_identification_start 1  
        30: aac_frame_length 13 length of the frame including header (in bytes) 
        43: adts_buffer_fullness 11 0x7FF indicates VBR 
        54: no_raw_data_blocks_in_frame 2  
        ADTS Error check 
        crc_check 16 only if protection_absent == 0 
   */
}