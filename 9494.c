cupsdFindCert(const char *certificate)	/* I - Certificate */
{
  cupsd_cert_t	*cert;			/* Current certificate */


  cupsdLogMessage(CUPSD_LOG_DEBUG2, "cupsdFindCert(certificate=%s)", certificate);
  for (cert = Certs; cert != NULL; cert = cert->next)
    if (!ctcompare(certificate, cert->certificate))
    {
      cupsdLogMessage(CUPSD_LOG_DEBUG2, "cupsdFindCert: Returning \"%s\".", cert->username);
      return (cert);
    }

  cupsdLogMessage(CUPSD_LOG_DEBUG2, "cupsdFindCert: Certificate not found.");

  return (NULL);
}