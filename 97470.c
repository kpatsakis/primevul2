static int audit_log_pid_context(struct audit_context *context, pid_t pid,
				 kuid_t auid, kuid_t uid, unsigned int sessionid,
				 u32 sid, char *comm)
{
	struct audit_buffer *ab;
	char *ctx = NULL;
	u32 len;
	int rc = 0;

	ab = audit_log_start(context, GFP_KERNEL, AUDIT_OBJ_PID);
	if (!ab)
		return rc;

	audit_log_format(ab, "opid=%d oauid=%d ouid=%d oses=%d", pid,
			 from_kuid(&init_user_ns, auid),
			 from_kuid(&init_user_ns, uid), sessionid);
	if (sid) {
		if (security_secid_to_secctx(sid, &ctx, &len)) {
			audit_log_format(ab, " obj=(none)");
			rc = 1;
		} else {
			audit_log_format(ab, " obj=%s", ctx);
			security_release_secctx(ctx, len);
		}
	}
	audit_log_format(ab, " ocomm=");
	audit_log_untrustedstring(ab, comm);
	audit_log_end(ab);

 	return rc;
 }
