static i32 ComparePictures(const void *ptr1, const void *ptr2)
{

/* Variables */

 dpbPicture_t *pic1, *pic2;

/* Code */

    ASSERT(ptr1);
    ASSERT(ptr2);

    pic1 = (dpbPicture_t*)ptr1;
    pic2 = (dpbPicture_t*)ptr2;

 /* both are non-reference pictures, check if needed for display */
 if (!IS_REFERENCE(*pic1) && !IS_REFERENCE(*pic2))
 {
 if (pic1->toBeDisplayed && !pic2->toBeDisplayed)
 return(-1);
 else if (!pic1->toBeDisplayed && pic2->toBeDisplayed)
 return(1);
 else
 return(0);
 }
 /* only pic 1 needed for reference -> greater */
 else if (!IS_REFERENCE(*pic2))
 return(-1);
 /* only pic 2 needed for reference -> greater */
 else if (!IS_REFERENCE(*pic1))
 return(1);
 /* both are short term reference pictures -> check picNum */
 else if (IS_SHORT_TERM(*pic1) && IS_SHORT_TERM(*pic2))
 {
 if (pic1->picNum > pic2->picNum)
 return(-1);
 else if (pic1->picNum < pic2->picNum)
 return(1);
 else
 return(0);
 }
 /* only pic 1 is short term -> greater */
 else if (IS_SHORT_TERM(*pic1))
 return(-1);
 /* only pic 2 is short term -> greater */
 else if (IS_SHORT_TERM(*pic2))
 return(1);
 /* both are long term reference pictures -> check picNum (contains the
     * longTermPicNum */
 else
 {
 if (pic1->picNum > pic2->picNum)
 return(1);
 else if (pic1->picNum < pic2->picNum)
 return(-1);
 else
 return(0);
 }
}
