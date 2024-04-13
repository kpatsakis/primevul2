int Exif2tm(struct tm * timeptr, char * ExifTime)
{
 int a;

    timeptr->tm_wday = -1;

    a = sscanf(ExifTime, "%d%*c%d%*c%d%*c%d:%d:%d",
 &timeptr->tm_year, &timeptr->tm_mon, &timeptr->tm_mday,
 &timeptr->tm_hour, &timeptr->tm_min, &timeptr->tm_sec);


 if (a == 6){
        timeptr->tm_isdst = -1;
        timeptr->tm_mon -= 1; // Adjust for unix zero-based months
        timeptr->tm_year -= 1900; // Adjust for year starting at 1900
 return TRUE; // worked.
 }

 return FALSE; // Wasn't in Exif date format.
}
