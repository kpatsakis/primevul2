static int cg_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	struct fuse_context *fc = fuse_get_context();
	struct file_info *f = (struct file_info *)fi->fh;
	struct cgfs_files *k = NULL;
	char *data = NULL;
	int ret, s;
	bool r;

	if (f->type != LXC_TYPE_CGFILE) {
		fprintf(stderr, "Internal error: directory cache info used in cg_read\n");
		return -EIO;
	}

	if (offset)
		return 0;

	if (!fc)
		return -EIO;

	if (!f->controller)
		return -EINVAL;

	if ((k = cgfs_get_key(f->controller, f->cgroup, f->file)) == NULL) {
		return -EINVAL;
	}
	free_key(k);


	if (!fc_may_access(fc, f->controller, f->cgroup, f->file, O_RDONLY)) { // should never get here
		ret = -EACCES;
		goto out;
	}

	if (strcmp(f->file, "tasks") == 0 ||
			strcmp(f->file, "/tasks") == 0 ||
			strcmp(f->file, "/cgroup.procs") == 0 ||
			strcmp(f->file, "cgroup.procs") == 0)
		r = do_read_pids(fc->pid, f->controller, f->cgroup, f->file, &data);
	else
		r = cgfs_get_value(f->controller, f->cgroup, f->file, &data);

	if (!r) {
		ret = -EINVAL;
		goto out;
	}

	if (!data) {
		ret = 0;
		goto out;
	}
	s = strlen(data);
	if (s > size)
		s = size;
	memcpy(buf, data, s);
	if (s > 0 && s < size && data[s-1] != '\n')
		buf[s++] = '\n';

	ret = s;

out:
	free(data);
	return ret;
}
