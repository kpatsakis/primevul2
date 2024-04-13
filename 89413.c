EAS_I16 ConvertRate (EAS_I32 timeCents)
{
    EAS_I32 temp;

 if (timeCents == ZERO_TIME_IN_CENTS)
 return SYNTH_FULL_SCALE_EG1_GAIN;

 /* divide frame rate by time in log domain to get rate */
    temp = dlsRateConvert - timeCents;

#if 1
    temp = EAS_Calculate2toX(temp);
#else
 /* convert from time cents to 10-bit fraction */
    temp = FMUL_15x15(temp, TIME_CENTS_TO_LOG2);

 /* convert to rate */
    temp = EAS_LogToLinear16(temp);
#endif

 if (temp < SYNTH_FULL_SCALE_EG1_GAIN)
 return (EAS_I16) temp;
 return SYNTH_FULL_SCALE_EG1_GAIN;
}
