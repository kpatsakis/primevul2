bsg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct bsg_device *bd = file->private_data;
	ssize_t bytes_written;
	int ret;
 
 	dprintk("%s: write %Zd bytes\n", bd->name, count);
 
	if (unlikely(segment_eq(get_fs(), KERNEL_DS)))
		return -EINVAL;

 	bsg_set_block(bd, file);
 
 	bytes_written = 0;
	ret = __bsg_write(bd, buf, count, &bytes_written,
			  file->f_mode & FMODE_WRITE);

	*ppos = bytes_written;

	/*
	 * return bytes written on non-fatal errors
	 */
	if (!bytes_written || err_block_err(ret))
		bytes_written = ret;

	dprintk("%s: returning %Zd\n", bd->name, bytes_written);
	return bytes_written;
}
