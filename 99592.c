compare_clients(cupsd_client_t *a,	/* I - First client */
                cupsd_client_t *b,	/* I - Second client */
                void           *data)	/* I - User data (not used) */
{
  (void)data;

  if (a == b)
    return (0);
  else if (a < b)
    return (-1);
  else
    return (1);
}
