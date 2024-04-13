SYSCALL_DEFINE2(setregid, gid_t, rgid, gid_t, egid)
{
	struct user_namespace *ns = current_user_ns();
	const struct cred *old;
	struct cred *new;
	int retval;
	kgid_t krgid, kegid;

	krgid = make_kgid(ns, rgid);
	kegid = make_kgid(ns, egid);

	if ((rgid != (gid_t) -1) && !gid_valid(krgid))
		return -EINVAL;
	if ((egid != (gid_t) -1) && !gid_valid(kegid))
		return -EINVAL;

	new = prepare_creds();
	if (!new)
		return -ENOMEM;
	old = current_cred();

	retval = -EPERM;
	if (rgid != (gid_t) -1) {
		if (gid_eq(old->gid, krgid) ||
		    gid_eq(old->egid, krgid) ||
		    nsown_capable(CAP_SETGID))
			new->gid = krgid;
		else
			goto error;
	}
	if (egid != (gid_t) -1) {
		if (gid_eq(old->gid, kegid) ||
		    gid_eq(old->egid, kegid) ||
		    gid_eq(old->sgid, kegid) ||
		    nsown_capable(CAP_SETGID))
			new->egid = kegid;
		else
			goto error;
	}

	if (rgid != (gid_t) -1 ||
	    (egid != (gid_t) -1 && !gid_eq(kegid, old->gid)))
		new->sgid = new->egid;
	new->fsgid = new->egid;

	return commit_creds(new);

error:
	abort_creds(new);
	return retval;
}
