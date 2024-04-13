SampleEncrypter::EncryptVideoSample(AP4_DataBuffer& sample, AP4_UI08 nalu_length_size)
{
    AP4_DataBuffer encrypted;
    
    AP4_UI08* nalu = sample.UseData();
    AP4_Size  bytes_remaining = sample.GetDataSize();
    while (bytes_remaining > nalu_length_size) {
        AP4_Size nalu_length = 0;
        switch (nalu_length_size) {
            case 1:
                nalu_length = nalu[0];
                break;
                
            case 2:
                nalu_length = AP4_BytesToUInt16BE(nalu);
                break;
    
            case 4:
                nalu_length = AP4_BytesToUInt32BE(nalu);
                break;
                
            default:
                break;
        }
        
        if (bytes_remaining < nalu_length_size+nalu_length) {
            break;
        }
        
        AP4_UI08 nalu_type = nalu[nalu_length_size] & 0x1F;
        if (nalu_length > 48 && (nalu_type == 1 || nalu_type == 5)) {
            AP4_Size encrypted_size = 16*((nalu_length-32)/16);
            if ((nalu_length%16) == 0) {
                encrypted_size -= 16;
            }
            m_StreamCipher->SetIV(m_IV);
            for (unsigned int i=0; i<encrypted_size; i += 10*16) {
                AP4_Size one_block_size = 16;
                m_StreamCipher->ProcessBuffer(nalu+nalu_length_size+32+i, one_block_size,
                                              nalu+nalu_length_size+32+i, &one_block_size);
            }

            // perform startcode emulation prevention
            AP4_DataBuffer escaped_nalu;
            PreventStartCodeEmulation(nalu+nalu_length_size, nalu_length, escaped_nalu);
            
            // the size may have changed
            // TODO: this could overflow if nalu_length_size is too small
            switch (nalu_length_size) {
                case 1:
                    nalu[0] = (AP4_UI08)(escaped_nalu.GetDataSize()&0xFF);
                    break;
                    
                case 2:
                    AP4_BytesFromUInt16BE(nalu, escaped_nalu.GetDataSize());
                    break;
        
                case 4:
                    AP4_BytesFromUInt32BE(nalu, escaped_nalu.GetDataSize());
                    break;
                    
                default:
                    break;
            }

            encrypted.AppendData(nalu, nalu_length_size);
            encrypted.AppendData(escaped_nalu.GetData(), escaped_nalu.GetDataSize());
        } else {
            encrypted.AppendData(nalu, nalu_length_size);
            encrypted.AppendData(nalu+nalu_length_size, nalu_length);
        }
        
        nalu            += nalu_length_size+nalu_length;
        bytes_remaining -= nalu_length_size+nalu_length;
    }
    
    sample.SetData(encrypted.GetData(), encrypted.GetDataSize());
    
    return AP4_SUCCESS;
}