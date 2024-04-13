cupsdDeleteCert(int pid)		/* I - Process ID */
{
  cupsd_cert_t	*cert,			/* Current certificate */
		*prev;			/* Previous certificate */
  char		filename[1024];		/* Certificate file */


  for (prev = NULL, cert = Certs; cert != NULL; prev = cert, cert = cert->next)
    if (cert->pid == pid)
    {
     /*
      * Remove this certificate from the list...
      */

      cupsdLogMessage(CUPSD_LOG_DEBUG2, "cupsdDeleteCert: Removing certificate for PID %d.", pid);

      if (prev == NULL)
        Certs = cert->next;
      else
        prev->next = cert->next;

      free(cert);

     /*
      * Delete the file and return...
      */

      snprintf(filename, sizeof(filename), "%s/certs/%d", StateDir, pid);
      if (unlink(filename))
	cupsdLogMessage(CUPSD_LOG_ERROR, "Unable to remove %s!", filename);

      return;
    }
}