 static int trusted_update(struct key *key, struct key_preparsed_payload *prep)
 {
	struct trusted_key_payload *p;
 	struct trusted_key_payload *new_p;
 	struct trusted_key_options *new_o;
 	size_t datalen = prep->datalen;
 	char *datablob;
 	int ret = 0;
 
	if (test_bit(KEY_FLAG_NEGATIVE, &key->flags))
		return -ENOKEY;
	p = key->payload.data[0];
 	if (!p->migratable)
 		return -EPERM;
 	if (datalen <= 0 || datalen > 32767 || !prep->data)
		return -EINVAL;

	datablob = kmalloc(datalen + 1, GFP_KERNEL);
	if (!datablob)
		return -ENOMEM;
	new_o = trusted_options_alloc();
	if (!new_o) {
		ret = -ENOMEM;
		goto out;
	}
	new_p = trusted_payload_alloc(key);
	if (!new_p) {
		ret = -ENOMEM;
		goto out;
	}

	memcpy(datablob, prep->data, datalen);
	datablob[datalen] = '\0';
	ret = datablob_parse(datablob, new_p, new_o);
	if (ret != Opt_update) {
		ret = -EINVAL;
		kfree(new_p);
		goto out;
	}

	if (!new_o->keyhandle) {
		ret = -EINVAL;
		kfree(new_p);
		goto out;
	}

	/* copy old key values, and reseal with new pcrs */
	new_p->migratable = p->migratable;
	new_p->key_len = p->key_len;
	memcpy(new_p->key, p->key, p->key_len);
	dump_payload(p);
	dump_payload(new_p);

	ret = key_seal(new_p, new_o);
	if (ret < 0) {
		pr_info("trusted_key: key_seal failed (%d)\n", ret);
		kfree(new_p);
		goto out;
	}
	if (new_o->pcrlock) {
		ret = pcrlock(new_o->pcrlock);
		if (ret < 0) {
			pr_info("trusted_key: pcrlock failed (%d)\n", ret);
			kfree(new_p);
			goto out;
		}
	}
	rcu_assign_keypointer(key, new_p);
	call_rcu(&p->rcu, trusted_rcu_free);
out:
	kfree(datablob);
	kfree(new_o);
	return ret;
}