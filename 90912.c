static bool set_one_prio_perm(struct task_struct *p)
{
 const struct cred *cred = current_cred(), *pcred = __task_cred(p);

 if (pcred->user->user_ns == cred->user->user_ns &&
 (pcred->uid  == cred->euid ||
	     pcred->euid == cred->euid))
 return true;
 if (ns_capable(pcred->user->user_ns, CAP_SYS_NICE))
 return true;
 return false;
}
