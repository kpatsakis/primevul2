void h264bsdFlushDpb(dpbStorage_t *dpb)
{

 /* don't do anything if buffer not reserved */
 if (dpb->buffer)
 {
        dpb->flushed = 1;
 /* output all pictures */
 while (OutputPicture(dpb) == HANTRO_OK)
 ;
 }

}
