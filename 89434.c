static EAS_RESULT PopcdlStack (EAS_U32 *pStack, EAS_INT *pStackPtr, EAS_U32 *pValue)
{

 /* stack underflow, cdl block has an errorr */
 if (*pStackPtr < 0)
 return EAS_ERROR_FILE_FORMAT;

 /* pop the value off the stack */
 *pValue = pStack[*pStackPtr];
 *pStackPtr = *pStackPtr - 1;
 return EAS_SUCCESS;
}
