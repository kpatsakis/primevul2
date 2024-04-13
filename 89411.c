static EAS_I8 ConvertPan (EAS_I32 pan)
{

 /* multiply by conversion factor */
    pan = FMUL_15x15 (PAN_CONVERSION_FACTOR, pan);
 if (pan < MIN_PAN_VALUE)
 return MIN_PAN_VALUE;
 if (pan > MAX_PAN_VALUE)
 return MAX_PAN_VALUE;
 return (EAS_I8) pan;
}
