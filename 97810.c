_SSL_close (SSL * ssl)
{
	SSL_set_shutdown (ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
 	SSL_free (ssl);
 	ERR_remove_state (0);		  /* free state buffer */
 }
