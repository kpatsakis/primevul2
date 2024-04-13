static int proc_stat_read(char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	struct fuse_context *fc = fuse_get_context();
	struct file_info *d = (struct file_info *)fi->fh;
	char *cg;
	char *cpuset = NULL;
	char *line = NULL;
	size_t linelen = 0, total_len = 0, rv = 0;
	int curcpu = -1; /* cpu numbering starts at 0 */
	unsigned long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0;
	unsigned long user_sum = 0, nice_sum = 0, system_sum = 0, idle_sum = 0, iowait_sum = 0,
					irq_sum = 0, softirq_sum = 0, steal_sum = 0, guest_sum = 0;
#define CPUALL_MAX_SIZE BUF_RESERVE_SIZE
	char cpuall[CPUALL_MAX_SIZE];
	/* reserve for cpu all */
	char *cache = d->buf + CPUALL_MAX_SIZE;
	size_t cache_size = d->buflen - CPUALL_MAX_SIZE;
	FILE *f = NULL;

	if (offset){
		if (offset > d->size)
			return -EINVAL;
		if (!d->cached)
			return 0;
		int left = d->size - offset;
		total_len = left > size ? size: left;
		memcpy(buf, d->buf + offset, total_len);
		return total_len;
	}

	cg = get_pid_cgroup(fc->pid, "cpuset");
	if (!cg)
		return read_file("/proc/stat", buf, size, d);

	cpuset = get_cpuset(cg);
	if (!cpuset)
		goto err;

	f = fopen("/proc/stat", "r");
	if (!f)
		goto err;

	if (getline(&line, &linelen, f) < 0) {
		fprintf(stderr, "proc_stat_read read first line failed\n");
		goto err;
	}

	while (getline(&line, &linelen, f) != -1) {
		size_t l;
		int cpu;
		char cpu_char[10]; /* That's a lot of cores */
		char *c;

		if (sscanf(line, "cpu%9[^ ]", cpu_char) != 1) {
			/* not a ^cpuN line containing a number N, just print it */
			l = snprintf(cache, cache_size, "%s", line);
			if (l < 0) {
				perror("Error writing to cache");
				rv = 0;
				goto err;
			}
			if (l >= cache_size) {
				fprintf(stderr, "Internal error: truncated write to cache\n");
				rv = 0;
				goto err;
			}
			if (l < cache_size) {
				cache += l;
				cache_size -= l;
				total_len += l;
				continue;
			} else {
				cache += cache_size;
				total_len += cache_size;
				cache_size = 0;
				break;
			}
		}

		if (sscanf(cpu_char, "%d", &cpu) != 1)
			continue;
		if (!cpu_in_cpuset(cpu, cpuset))
			continue;
		curcpu ++;

		c = strchr(line, ' ');
		if (!c)
			continue;
		l = snprintf(cache, cache_size, "cpu%d%s", curcpu, c);
		if (l < 0) {
			perror("Error writing to cache");
			rv = 0;
			goto err;

		}
		if (l >= cache_size) {
			fprintf(stderr, "Internal error: truncated write to cache\n");
			rv = 0;
			goto err;
		}

		cache += l;
		cache_size -= l;
		total_len += l;

		if (sscanf(line, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq,
			&softirq, &steal, &guest) != 9)
			continue;
		user_sum += user;
		nice_sum += nice;
		system_sum += system;
		idle_sum += idle;
		iowait_sum += iowait;
		irq_sum += irq;
		softirq_sum += softirq;
		steal_sum += steal;
		guest_sum += guest;
	}

	cache = d->buf;

	int cpuall_len = snprintf(cpuall, CPUALL_MAX_SIZE, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
		"cpu ", user_sum, nice_sum, system_sum, idle_sum, iowait_sum, irq_sum, softirq_sum, steal_sum, guest_sum);
	if (cpuall_len > 0 && cpuall_len < CPUALL_MAX_SIZE){
		memcpy(cache, cpuall, cpuall_len);
		cache += cpuall_len;
	} else{
		/* shouldn't happen */
		fprintf(stderr, "proc_stat_read copy cpuall failed, cpuall_len=%d\n", cpuall_len);
		cpuall_len = 0;
	}

	memmove(cache, d->buf + CPUALL_MAX_SIZE, total_len);
	total_len += cpuall_len;
	d->cached = 1;
	d->size = total_len;
	if (total_len > size ) total_len = size;

	memcpy(buf, d->buf, total_len);
	rv = total_len;

err:
	if (f)
		fclose(f);
	free(line);
	free(cpuset);
	free(cg);
	return rv;
}
