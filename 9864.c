WriteAc4Header(AP4_ByteStream& output,
                unsigned int    frame_size)
{
    unsigned char bits[7];

    bits[0] = 0xac;
    bits[1] = 0x40;
    bits[2] = 0xff;
    bits[3] = 0xff;
    bits[4] = (frame_size>>16)&0xFF;
    bits[5] = (frame_size>>8 )&0xFF;
    bits[6] = (frame_size    )&0xFF;

    return output.Write(bits, 7);
}