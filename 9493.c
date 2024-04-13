cupsdInitCerts(void)
{
#ifndef HAVE_ARC4RANDOM
  cups_file_t	*fp;			/* /dev/random file */


 /*
  * Initialize the random number generator using the random device or
  * the current time, as available...
  */

  if ((fp = cupsFileOpen("/dev/urandom", "rb")) == NULL)
  {
    struct timeval tod;			/* Time of day */

   /*
    * Get the time in usecs and use it as the initial seed...
    */

    gettimeofday(&tod, NULL);

    CUPS_SRAND((unsigned)(tod.tv_sec + tod.tv_usec));
  }
  else
  {
    unsigned	seed;			/* Seed for random number generator */

   /*
    * Read 4 random characters from the random device and use
    * them as the seed...
    */

    seed = (unsigned)cupsFileGetChar(fp);
    seed = (seed << 8) | (unsigned)cupsFileGetChar(fp);
    seed = (seed << 8) | (unsigned)cupsFileGetChar(fp);
    CUPS_SRAND((seed << 8) | (unsigned)cupsFileGetChar(fp));

    cupsFileClose(fp);
  }
#endif /* !HAVE_ARC4RANDOM */

 /*
  * Create a root certificate and return...
  */

  if (!RunUser)
    cupsdAddCert(0, "root", cupsdDefaultAuthType());
}