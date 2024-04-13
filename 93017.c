static int snd_timer_start_slave(struct snd_timer_instance *timeri)
{
	unsigned long flags;
 
 	spin_lock_irqsave(&slave_active_lock, flags);
 	timeri->flags |= SNDRV_TIMER_IFLG_RUNNING;
	if (timeri->master && timeri->timer) {
		spin_lock(&timeri->timer->lock);
 		list_add_tail(&timeri->active_list,
 			      &timeri->master->slave_active_head);
		spin_unlock(&timeri->timer->lock);
	}
 	spin_unlock_irqrestore(&slave_active_lock, flags);
 	return 1; /* delayed start */
 }