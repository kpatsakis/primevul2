nvmet_fc_find_target_queue(struct nvmet_fc_tgtport *tgtport,
				u64 connection_id)
{
	struct nvmet_fc_tgt_assoc *assoc;
	struct nvmet_fc_tgt_queue *queue;
	u64 association_id = nvmet_fc_getassociationid(connection_id);
 	u16 qid = nvmet_fc_getqueueid(connection_id);
 	unsigned long flags;
 
	if (qid > NVMET_NR_QUEUES)
		return NULL;

 	spin_lock_irqsave(&tgtport->lock, flags);
 	list_for_each_entry(assoc, &tgtport->assoc_list, a_list) {
 		if (association_id == assoc->association_id) {
			queue = assoc->queues[qid];
			if (queue &&
			    (!atomic_read(&queue->connected) ||
			     !nvmet_fc_tgt_q_get(queue)))
				queue = NULL;
			spin_unlock_irqrestore(&tgtport->lock, flags);
			return queue;
		}
	}
	spin_unlock_irqrestore(&tgtport->lock, flags);
	return NULL;
}
