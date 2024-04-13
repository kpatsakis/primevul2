static u32 SlidingWindowRefPicMarking(dpbStorage_t *dpb)
{

/* Variables */

    i32 index, picNum;
    u32 i;

/* Code */

 if (dpb->numRefFrames < dpb->maxRefFrames)
 {
 return(HANTRO_OK);
 }
 else
 {
        index = -1;
        picNum = 0;
 /* find the oldest short term picture */
 for (i = 0; i < dpb->numRefFrames; i++)
 if (IS_SHORT_TERM(dpb->buffer[i]))
 if (dpb->buffer[i].picNum < picNum || index == -1)
 {
                    index = (i32)i;
                    picNum = dpb->buffer[i].picNum;
 }
 if (index >= 0)
 {
            SET_UNUSED(dpb->buffer[index]);
            dpb->numRefFrames--;
 if (!dpb->buffer[index].toBeDisplayed)
                dpb->fullness--;

 return(HANTRO_OK);
 }
 }

 return(HANTRO_NOK);

}
