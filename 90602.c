WORD16 impeg2d_dec_vld_symbol(stream_t *ps_stream,const WORD16 ai2_code_table[][2],  UWORD16 u2_max_len)
{
  UWORD16 u2_data;
  WORD16  u2_end = 0;
  UWORD16 u2_org_max_len = u2_max_len;
  UWORD16 u2_i_bit;

 /* Get the maximum number of bits needed to decode a symbol */
  u2_data = impeg2d_bit_stream_nxt(ps_stream,u2_max_len);
 do
 {
    u2_max_len--;
 /* Read one bit at a time from the variable to decode the huffman code */
    u2_i_bit = (UWORD8)((u2_data >> u2_max_len) & 0x1);

 /* Get the next node pointer or the symbol from the tree */
    u2_end = ai2_code_table[u2_end][u2_i_bit];
 }while(u2_end > 0);

 /* Flush the appropriate number of bits from the ps_stream */
  impeg2d_bit_stream_flush(ps_stream,(UWORD8)(u2_org_max_len - u2_max_len));
 return(u2_end);
}
