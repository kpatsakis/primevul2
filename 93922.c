 static void ptrace_link(struct task_struct *child, struct task_struct *new_parent)
 {
	__ptrace_link(child, new_parent, current_cred());
 }
