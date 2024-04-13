static void sample_to_timespec(const clockid_t which_clock,
 			       union cpu_time_count cpu,
 			       struct timespec *tp)
 {
	if (CPUCLOCK_WHICH(which_clock) == CPUCLOCK_SCHED)
		*tp = ns_to_timespec(cpu.sched);
	else
 		cputime_to_timespec(cpu.cpu, tp);
 }
