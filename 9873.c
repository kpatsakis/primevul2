PackedAudioWriter::WriteHeader(double          timestamp,
                               const char*     private_extension_name,
                               const AP4_UI08* private_extension_data,
                               unsigned int    private_extension_data_size,
                               AP4_ByteStream& output)
{
    unsigned int header_size = 10+10+45+8;
    unsigned int private_extension_name_size = private_extension_name?(unsigned int)AP4_StringLength(private_extension_name)+1:0;
    if (private_extension_name) {
        header_size += 10+private_extension_name_size+private_extension_data_size;;
    }
    AP4_UI08* header = new AP4_UI08[header_size];
    
    /* ID3 Tag Header
     ID3v2/file identifier      "ID3"
     ID3v2 version              $04 00
     ID3v2 flags                %abcd0000
     ID3v2 size                 4 * %0xxxxxxx
    */
    header[0] = 'I';
    header[1] = 'D';
    header[2] = '3';
    header[3] = 4;
    header[4] = 0;
    header[5] = 0;
    header[6] = 0;
    header[7] = 0;
    header[8] = ((header_size-10)>>7) & 0x7F;
    header[9] =  (header_size-10)     & 0x7F;
    
    /* PRIV frame
     Frame ID   $xx xx xx xx  (four characters)
     Size       $xx xx xx xx
     Flags      $xx xx
     
     Owner identifier      <text string> $00
     The private data      <binary data>    
    */
    header[10] = 'P';
    header[11] = 'R';
    header[12] = 'I';
    header[13] = 'V';
    header[14] = 0;
    header[15] = 0;
    header[16] = 0;
    header[17] = 45+8;
    header[18] = 0;
    header[19] = 0;
    AP4_CopyMemory(&header[20], "com.apple.streaming.transportStreamTimestamp", 45);
    AP4_UI64 mpeg_ts = (AP4_UI64)(timestamp*90000.0);
    AP4_BytesFromUInt64BE(&header[10+10+45], mpeg_ts);

    // add the extra private extension if needed
    if (private_extension_name) {
        AP4_UI08* ext = &header[10+10+45+8];
        ext[0] = 'P';
        ext[1] = 'R';
        ext[2] = 'I';
        ext[3] = 'V';
        ext[4] = 0;
        ext[5] = 0;
        ext[6] = ((private_extension_name_size+private_extension_data_size)>>7) & 0x7F;
        ext[7] =  (private_extension_name_size+private_extension_data_size)     & 0x7F;
        ext[8] = 0;
        ext[9] = 0;
        AP4_CopyMemory(&ext[10], private_extension_name, private_extension_name_size);
        AP4_CopyMemory(&ext[10+private_extension_name_size], private_extension_data, private_extension_data_size);
    }
    
    // write the header out
    AP4_Result result = output.Write(header, header_size);
    delete[] header;
    
    return result;
}