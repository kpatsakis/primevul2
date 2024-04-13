static void put_ucounts(struct ucounts *ucounts)
 {
 	unsigned long flags;
 
	spin_lock_irqsave(&ucounts_lock, flags);
	ucounts->count -= 1;
	if (!ucounts->count)
 		hlist_del_init(&ucounts->node);
	else
		ucounts = NULL;
	spin_unlock_irqrestore(&ucounts_lock, flags);
 
	kfree(ucounts);
 }
