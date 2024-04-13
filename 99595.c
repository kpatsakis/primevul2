is_cgi(cupsd_client_t *con,		/* I - Client connection */
       const char     *filename,	/* I - Real filename */
       struct stat    *filestats,	/* I - File information */
       mime_type_t    *type)		/* I - MIME type */
{
  const char	*options;		/* Options on URL */


 /*
  * Get the options, if any...
  */

  if ((options = strchr(con->uri, '?')) != NULL)
  {
    options ++;
    cupsdSetStringf(&(con->query_string), "QUERY_STRING=%s", options);
  }

 /*
  * Check for known types...
  */

  if (!type || _cups_strcasecmp(type->super, "application"))
  {
    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 0.", filename, filestats, type ? type->super : "unknown", type ? type->type : "unknown");
    return (0);
  }

  if (!_cups_strcasecmp(type->type, "x-httpd-cgi") &&
      (filestats->st_mode & 0111))
  {
   /*
    * "application/x-httpd-cgi" is a CGI script.
    */

    cupsdSetString(&con->command, filename);

    if (options)
      cupsdSetStringf(&con->options, " %s", options);

    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 1.", filename, filestats, type->super, type->type);
    return (1);
  }
#ifdef HAVE_JAVA
  else if (!_cups_strcasecmp(type->type, "x-httpd-java"))
  {
   /*
    * "application/x-httpd-java" is a Java servlet.
    */

    cupsdSetString(&con->command, CUPS_JAVA);

    if (options)
      cupsdSetStringf(&con->options, " %s %s", filename, options);
    else
      cupsdSetStringf(&con->options, " %s", filename);

    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 1.", filename, filestats, type->super, type->type);
    return (1);
  }
#endif /* HAVE_JAVA */
#ifdef HAVE_PERL
  else if (!_cups_strcasecmp(type->type, "x-httpd-perl"))
  {
   /*
    * "application/x-httpd-perl" is a Perl page.
    */

    cupsdSetString(&con->command, CUPS_PERL);

    if (options)
      cupsdSetStringf(&con->options, " %s %s", filename, options);
    else
      cupsdSetStringf(&con->options, " %s", filename);

    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 1.", filename, filestats, type->super, type->type);
    return (1);
  }
#endif /* HAVE_PERL */
#ifdef HAVE_PHP
  else if (!_cups_strcasecmp(type->type, "x-httpd-php"))
  {
   /*
    * "application/x-httpd-php" is a PHP page.
    */

    cupsdSetString(&con->command, CUPS_PHP);

    if (options)
      cupsdSetStringf(&con->options, " %s %s", filename, options);
    else
      cupsdSetStringf(&con->options, " %s", filename);

    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 1.", filename, filestats, type->super, type->type);
    return (1);
  }
#endif /* HAVE_PHP */
#ifdef HAVE_PYTHON
  else if (!_cups_strcasecmp(type->type, "x-httpd-python"))
  {
   /*
    * "application/x-httpd-python" is a Python page.
    */

    cupsdSetString(&con->command, CUPS_PYTHON);

    if (options)
      cupsdSetStringf(&con->options, " %s %s", filename, options);
    else
      cupsdSetStringf(&con->options, " %s", filename);

    cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 1.", filename, filestats, type->super, type->type);
    return (1);
  }
#endif /* HAVE_PYTHON */

  cupsdLogClient(con, CUPSD_LOG_DEBUG2, "is_cgi: filename=\"%s\", filestats=%p, type=%s/%s, returning 0.", filename, filestats, type->super, type->type);
  return (0);
}
