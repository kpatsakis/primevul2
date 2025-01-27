static void tg3_stop(struct tg3 *tp)
{
	int i;

	tg3_reset_task_cancel(tp);
	tg3_netif_stop(tp);

	tg3_timer_stop(tp);

	tg3_hwmon_close(tp);

	tg3_phy_stop(tp);

	tg3_full_lock(tp, 1);

	tg3_disable_ints(tp);

	tg3_halt(tp, RESET_KIND_SHUTDOWN, 1);
	tg3_free_rings(tp);
	tg3_flag_clear(tp, INIT_COMPLETE);

	tg3_full_unlock(tp);

	for (i = tp->irq_cnt - 1; i >= 0; i--) {
		struct tg3_napi *tnapi = &tp->napi[i];
		free_irq(tnapi->irq_vec, tnapi);
	}

	tg3_ints_fini(tp);

	tg3_napi_fini(tp);

	tg3_free_consistent(tp);
}
