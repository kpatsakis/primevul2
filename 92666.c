cifs_find_smb_ses(struct TCP_Server_Info *server, char *username)
cifs_find_smb_ses(struct TCP_Server_Info *server, struct smb_vol *vol)
 {
 	struct cifsSesInfo *ses;
 
 	write_lock(&cifs_tcp_ses_lock);
	list_for_each_entry(ses, &server->smb_ses_list, smb_ses_list) {
		switch (server->secType) {
		case Kerberos:
			if (vol->linux_uid != ses->linux_uid)
				continue;
			break;
		default:
			/* anything else takes username/password */
			if (strncmp(ses->userName, vol->username,
				    MAX_USERNAME_SIZE))
				continue;
			if (strlen(vol->username) != 0 &&
			    strncmp(ses->password, vol->password,
				    MAX_PASSWORD_SIZE))
				continue;
		}
 		++ses->ses_count;
 		write_unlock(&cifs_tcp_ses_lock);
 		return ses;
	}
	write_unlock(&cifs_tcp_ses_lock);
	return NULL;
}
