SYSCALL_DEFINE3(setpriority, int, which, int, who, int, niceval)
{
 struct task_struct *g, *p;
 struct user_struct *user;
 const struct cred *cred = current_cred();
 int error = -EINVAL;
 struct pid *pgrp;

 if (which > PRIO_USER || which < PRIO_PROCESS)
 goto out;

 /* normalize: avoid signed division (rounding problems) */
	error = -ESRCH;
 if (niceval < -20)
		niceval = -20;
 if (niceval > 19)
		niceval = 19;

	rcu_read_lock();
	read_lock(&tasklist_lock);
 switch (which) {
 case PRIO_PROCESS:
 if (who)
				p = find_task_by_vpid(who);
 else
				p = current;
 if (p)
				error = set_one_prio(p, niceval, error);
 break;
 case PRIO_PGRP:
 if (who)
				pgrp = find_vpid(who);
 else
				pgrp = task_pgrp(current);
			do_each_pid_thread(pgrp, PIDTYPE_PGID, p) {
				error = set_one_prio(p, niceval, error);
 } while_each_pid_thread(pgrp, PIDTYPE_PGID, p);
 break;
 case PRIO_USER:
			user = (struct user_struct *) cred->user;
 if (!who)
				who = cred->uid;
 else if ((who != cred->uid) &&
 !(user = find_user(who)))
 goto out_unlock; /* No processes for this user */

			do_each_thread(g, p) {
 if (__task_cred(p)->uid == who)
					error = set_one_prio(p, niceval, error);
 } while_each_thread(g, p);
 if (who != cred->uid)
				free_uid(user); /* For find_user() */
 break;
 }
out_unlock:
	read_unlock(&tasklist_lock);
	rcu_read_unlock();
out:
 return error;
}
