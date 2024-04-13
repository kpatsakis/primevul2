static u32 Mmcop5(dpbStorage_t *dpb)
{

/* Variables */

    u32 i;

/* Code */

 for (i = 0; i < 16; i++)
 {
 if (IS_REFERENCE(dpb->buffer[i]))
 {
            SET_UNUSED(dpb->buffer[i]);
 if (!dpb->buffer[i].toBeDisplayed)
                dpb->fullness--;
 }
 }

 /* output all pictures */
 while (OutputPicture(dpb) == HANTRO_OK)
 ;
    dpb->numRefFrames = 0;
    dpb->maxLongTermFrameIdx = NO_LONG_TERM_FRAME_INDICES;
    dpb->prevRefFrameNum = 0;

 return(HANTRO_OK);

}
