uuid_make(char *str)
{
  uint16_t uuid[8];
  time_t now;
  int i;

  now = time(NULL);

  srand((unsigned int)now);

  for (i = 0; i < ARRAY_SIZE(uuid); i++)
    {
      uuid[i] = (uint16_t)rand();

      // time_hi_and_version, set version to 4 (=random)
      if (i == 3)
	uuid[i] = (uuid[i] & 0x0FFF) | 0x4000;
      // clock_seq, variant 1
      if (i == 4)
	uuid[i] = (uuid[i] & 0x3FFF) | 0x8000;


      if (i == 2 || i == 3 || i == 4 || i == 5)
	str += sprintf(str, "-");

      str += sprintf(str, "%04" PRIX16, uuid[i]);
    }
}