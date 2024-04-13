UWORD16 impeg2d_dec_ac_coeff_zero(stream_t *ps_stream, UWORD16* pu2_sym_len, UWORD16* pu2_sym_val)
{
    UWORD16 u2_offset,u2_decoded_value;
    UWORD8  u1_shift;
    UWORD32 u4_bits_read;

    u4_bits_read = (UWORD16)impeg2d_bit_stream_nxt(ps_stream,MPEG2_AC_COEFF_MAX_LEN);

 if ((UWORD16)u4_bits_read >= 0x0800)
 {
        u2_offset = (UWORD16)u4_bits_read >> 11;
 }
 else if ((UWORD16)u4_bits_read >= 0x40)
 {
        u2_offset = 31 + ((UWORD16)u4_bits_read >> 6);
 }
 else if ((UWORD16)u4_bits_read >= 0x20)
 {
        u2_offset = 64;
 }
 else
 {
        u2_offset      = 63;
        u4_bits_read    = (UWORD16)u4_bits_read - 0x10;
 }
 /*-----------------------------------------------------------------------
     * The table gOffset contains both the offset for the group to which the
     * Vld code belongs in the Ac Coeff Table and the no of bits with which
     * the BitsRead should be shifted
     *-----------------------------------------------------------------------*/
    u2_offset = gau2_impeg2d_offset_zero[u2_offset];
    u1_shift  = u2_offset & 0xF;

 /*-----------------------------------------------------------------------
     * Depending upon the vld code, we index exactly to that particular
     * Vld codes value in the Ac Coeff Table.
     * (Offset >> 4)       gives the offset for the group in the AcCoeffTable.
     * (BitsRead >> shift) gives the offset within its group
     *-----------------------------------------------------------------------*/
     u2_offset = (u2_offset >> 4) + ((UWORD16)u4_bits_read >> u1_shift);
 /*-----------------------------------------------------------------------
     * DecodedValue has the Run, Level and the number of bits used by Vld code
     *-----------------------------------------------------------------------*/
    u2_decoded_value = gau2_impeg2d_dct_coeff_zero[u2_offset];
 if(u2_decoded_value == END_OF_BLOCK)
 {
 *pu2_sym_len = 2;
 *pu2_sym_val = EOB_CODE_VALUE;
 }
 else if(u2_decoded_value == ESCAPE_CODE)
 {
 *pu2_sym_len     = u2_decoded_value & 0x1F;
 *pu2_sym_val = ESC_CODE_VALUE;
 }
 else
 {
 *pu2_sym_len = u2_decoded_value & 0x1F;
 *pu2_sym_val = u2_decoded_value >> 5;
 }
 return(u2_decoded_value);
}
