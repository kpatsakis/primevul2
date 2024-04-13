cupsdAddCert(int        pid,		/* I - Process ID */
             const char *username,	/* I - Username */
             int        type)		/* I - AuthType for username */
{
  int		i;			/* Looping var */
  cupsd_cert_t	*cert;			/* Current certificate */
  int		fd;			/* Certificate file */
  char		filename[1024];		/* Certificate filename */
  static const char hex[] = "0123456789ABCDEF";
					/* Hex constants... */


  cupsdLogMessage(CUPSD_LOG_DEBUG, "cupsdAddCert: Adding certificate for PID %d", pid);

 /*
  * Allocate memory for the certificate...
  */

  if ((cert = calloc(sizeof(cupsd_cert_t), 1)) == NULL)
    return;

 /*
  * Fill in the certificate information...
  */

  cert->pid  = pid;
  cert->type = type;
  strlcpy(cert->username, username, sizeof(cert->username));

  for (i = 0; i < 32; i ++)
    cert->certificate[i] = hex[CUPS_RAND() & 15];

 /*
  * Save the certificate to a file readable only by the User and Group
  * (or root and SystemGroup for PID == 0)...
  */

  snprintf(filename, sizeof(filename), "%s/certs/%d", StateDir, pid);
  unlink(filename);

  if ((fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0400)) < 0)
  {
    cupsdLogMessage(CUPSD_LOG_ERROR,
                    "Unable to create certificate file %s - %s",
                    filename, strerror(errno));
    free(cert);
    return;
  }

  if (pid == 0)
  {
#ifdef HAVE_ACL_INIT
    acl_t		acl;		/* ACL information */
    acl_entry_t		entry;		/* ACL entry */
    acl_permset_t	permset;	/* Permissions */
#  ifdef HAVE_MBR_UID_TO_UUID
    uuid_t		group;		/* Group ID */
#  endif /* HAVE_MBR_UID_TO_UUID */
    static int		acls_not_supported = 0;
					/* Only warn once */
#endif /* HAVE_ACL_INIT */


   /*
    * Root certificate...
    */

    fchmod(fd, 0440);
    fchown(fd, RunUser, SystemGroupIDs[0]);

    cupsdLogMessage(CUPSD_LOG_DEBUG2, "cupsdAddCert: NumSystemGroups=%d", NumSystemGroups);

#ifdef HAVE_ACL_INIT
    if (NumSystemGroups > 1)
    {
     /*
      * Set POSIX ACLs for the root certificate so that all system
      * groups can access it...
      */

      int	j;			/* Looping var */

#  ifdef HAVE_MBR_UID_TO_UUID
     /*
      * On macOS, ACLs use UUIDs instead of GIDs...
      */

      acl = acl_init(NumSystemGroups - 1);

      for (i = 1; i < NumSystemGroups; i ++)
      {
       /*
        * Add each group ID to the ACL...
	*/

        for (j = 0; j < i; j ++)
	  if (SystemGroupIDs[j] == SystemGroupIDs[i])
            break;

        if (j < i)
          continue;			/* Skip duplicate groups */

        acl_create_entry(&acl, &entry);
	acl_get_permset(entry, &permset);
	acl_add_perm(permset, ACL_READ_DATA);
	acl_set_tag_type(entry, ACL_EXTENDED_ALLOW);
	mbr_gid_to_uuid((gid_t)SystemGroupIDs[i], group);
	acl_set_qualifier(entry, &group);
	acl_set_permset(entry, permset);
      }

#  else
     /*
      * POSIX ACLs need permissions for owner, group, other, and mask
      * in addition to the rest of the system groups...
      */

      acl = acl_init(NumSystemGroups + 3);

      /* Owner */
      acl_create_entry(&acl, &entry);
      acl_get_permset(entry, &permset);
      acl_add_perm(permset, ACL_READ);
      acl_set_tag_type(entry, ACL_USER_OBJ);
      acl_set_permset(entry, permset);

      /* Group */
      acl_create_entry(&acl, &entry);
      acl_get_permset(entry, &permset);
      acl_add_perm(permset, ACL_READ);
      acl_set_tag_type(entry, ACL_GROUP_OBJ);
      acl_set_permset(entry, permset);

      /* Others */
      acl_create_entry(&acl, &entry);
      acl_get_permset(entry, &permset);
      acl_add_perm(permset, 0);
      acl_set_tag_type(entry, ACL_OTHER);
      acl_set_permset(entry, permset);

      /* Mask */
      acl_create_entry(&acl, &entry);
      acl_get_permset(entry, &permset);
      acl_add_perm(permset, ACL_READ);
      acl_set_tag_type(entry, ACL_MASK);
      acl_set_permset(entry, permset);

      for (i = 1; i < NumSystemGroups; i ++)
      {
       /*
        * Add each group ID to the ACL...
	*/

        for (j = 0; j < i; j ++)
	  if (SystemGroupIDs[j] == SystemGroupIDs[i])
            break;

        if (j < i)
          continue;			/* Skip duplicate groups */

        acl_create_entry(&acl, &entry);
	acl_get_permset(entry, &permset);
	acl_add_perm(permset, ACL_READ);
	acl_set_tag_type(entry, ACL_GROUP);
	acl_set_qualifier(entry, SystemGroupIDs + i);
	acl_set_permset(entry, permset);
      }

      if (acl_valid(acl))
      {
        char *text, *textptr;		/* Temporary string */

        cupsdLogMessage(CUPSD_LOG_ERROR, "ACL did not validate: %s",
	                strerror(errno));
        text = acl_to_text(acl, NULL);
	for (textptr = strchr(text, '\n');
	     textptr;
	     textptr = strchr(textptr + 1, '\n'))
	  *textptr = ',';

	cupsdLogMessage(CUPSD_LOG_ERROR, "ACL: %s", text);
	acl_free(text);
      }
#  endif /* HAVE_MBR_UID_TO_UUID */

      if (acl_set_fd(fd, acl))
      {
	if (errno != EOPNOTSUPP || !acls_not_supported)
	  cupsdLogMessage(CUPSD_LOG_ERROR,
			  "Unable to set ACLs on root certificate \"%s\" - %s",
			  filename, strerror(errno));

	if (errno == EOPNOTSUPP)
	  acls_not_supported = 1;
      }

      acl_free(acl);
    }
#endif /* HAVE_ACL_INIT */

    RootCertTime = time(NULL);
  }
  else
  {
   /*
    * CGI certificate...
    */

    fchmod(fd, 0400);
    fchown(fd, User, Group);
  }

  write(fd, cert->certificate, strlen(cert->certificate));
  close(fd);

 /*
  * Insert the certificate at the front of the list...
  */

  cert->next = Certs;
  Certs      = cert;
}