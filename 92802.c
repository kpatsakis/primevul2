static int newque(struct ipc_namespace *ns, struct ipc_params *params)
{
	struct msg_queue *msq;
	int id, retval;
	key_t key = params->key;
	int msgflg = params->flg;

	msq = ipc_rcu_alloc(sizeof(*msq));
	if (!msq)
		return -ENOMEM;

	msq->q_perm.mode = msgflg & S_IRWXUGO;
	msq->q_perm.key = key;

	msq->q_perm.security = NULL;
	retval = security_msg_queue_alloc(msq);
	if (retval) {
		ipc_rcu_putref(msq, ipc_rcu_free);
 		return retval;
 	}
 
 	msq->q_stime = msq->q_rtime = 0;
 	msq->q_ctime = get_seconds();
 	msq->q_cbytes = msq->q_qnum = 0;
	msq->q_qbytes = ns->msg_ctlmnb;
	msq->q_lspid = msq->q_lrpid = 0;
	INIT_LIST_HEAD(&msq->q_messages);
 	INIT_LIST_HEAD(&msq->q_receivers);
 	INIT_LIST_HEAD(&msq->q_senders);
 
	/* ipc_addid() locks msq upon success. */
	id = ipc_addid(&msg_ids(ns), &msq->q_perm, ns->msg_ctlmni);
	if (id < 0) {
		ipc_rcu_putref(msq, msg_rcu_free);
		return id;
	}

 	ipc_unlock_object(&msq->q_perm);
 	rcu_read_unlock();
 
	return msq->q_perm.id;
}
