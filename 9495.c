cupsdDeleteAllCerts(void)
{
  cupsd_cert_t	*cert,			/* Current certificate */
		*next;			/* Next certificate */
  char		filename[1024];		/* Certificate file */


 /*
  * Loop through each certificate, deleting them...
  */

  for (cert = Certs; cert != NULL; cert = next)
  {
   /*
    * Delete the file...
    */

    snprintf(filename, sizeof(filename), "%s/certs/%d", StateDir, cert->pid);
    if (unlink(filename))
      cupsdLogMessage(CUPSD_LOG_ERROR, "Unable to remove %s!", filename);

   /*
    * Free memory...
    */

    next = cert->next;
    free(cert);
  }

  Certs        = NULL;
  RootCertTime = 0;
}