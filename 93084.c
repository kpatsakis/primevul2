static void construct_get_dest_keyring(struct key **_dest_keyring)
static int construct_get_dest_keyring(struct key **_dest_keyring)
 {
 	struct request_key_auth *rka;
 	const struct cred *cred = current_cred();
 	struct key *dest_keyring = *_dest_keyring, *authkey;
	int ret;
 
 	kenter("%p", dest_keyring);
 
	/* find the appropriate keyring */
	if (dest_keyring) {
 		/* the caller supplied one */
 		key_get(dest_keyring);
 	} else {
		bool do_perm_check = true;

 		/* use a default keyring; falling through the cases until we
 		 * find one that we actually have */
 		switch (cred->jit_keyring) {
		case KEY_REQKEY_DEFL_DEFAULT:
		case KEY_REQKEY_DEFL_REQUESTOR_KEYRING:
			if (cred->request_key_auth) {
				authkey = cred->request_key_auth;
				down_read(&authkey->sem);
				rka = authkey->payload.data[0];
				if (!test_bit(KEY_FLAG_REVOKED,
					      &authkey->flags))
 					dest_keyring =
 						key_get(rka->dest_keyring);
 				up_read(&authkey->sem);
				if (dest_keyring) {
					do_perm_check = false;
 					break;
				}
 			}
 
 		case KEY_REQKEY_DEFL_THREAD_KEYRING:
			dest_keyring = key_get(cred->thread_keyring);
			if (dest_keyring)
				break;

		case KEY_REQKEY_DEFL_PROCESS_KEYRING:
			dest_keyring = key_get(cred->process_keyring);
			if (dest_keyring)
				break;

		case KEY_REQKEY_DEFL_SESSION_KEYRING:
			rcu_read_lock();
			dest_keyring = key_get(
				rcu_dereference(cred->session_keyring));
			rcu_read_unlock();

			if (dest_keyring)
				break;

		case KEY_REQKEY_DEFL_USER_SESSION_KEYRING:
			dest_keyring =
				key_get(cred->user->session_keyring);
			break;

		case KEY_REQKEY_DEFL_USER_KEYRING:
			dest_keyring = key_get(cred->user->uid_keyring);
			break;

		case KEY_REQKEY_DEFL_GROUP_KEYRING:
 		default:
 			BUG();
 		}

		/*
		 * Require Write permission on the keyring.  This is essential
		 * because the default keyring may be the session keyring, and
		 * joining a keyring only requires Search permission.
		 *
		 * However, this check is skipped for the "requestor keyring" so
		 * that /sbin/request-key can itself use request_key() to add
		 * keys to the original requestor's destination keyring.
		 */
		if (dest_keyring && do_perm_check) {
			ret = key_permission(make_key_ref(dest_keyring, 1),
					     KEY_NEED_WRITE);
			if (ret) {
				key_put(dest_keyring);
				return ret;
			}
		}
 	}
 
 	*_dest_keyring = dest_keyring;
 	kleave(" [dk %d]", key_serial(dest_keyring));
	return 0;
 }
