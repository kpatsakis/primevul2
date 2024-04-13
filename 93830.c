static void f2fs_put_super(struct super_block *sb)
{
	struct f2fs_sb_info *sbi = F2FS_SB(sb);
	int i;

	f2fs_quota_off_umount(sb);

	/* prevent remaining shrinker jobs */
	mutex_lock(&sbi->umount_mutex);

	/*
	 * We don't need to do checkpoint when superblock is clean.
	 * But, the previous checkpoint was not done by umount, it needs to do
	 * clean checkpoint again.
	 */
	if (is_sbi_flag_set(sbi, SBI_IS_DIRTY) ||
			!is_set_ckpt_flags(sbi, CP_UMOUNT_FLAG)) {
		struct cp_control cpc = {
			.reason = CP_UMOUNT,
		};
		write_checkpoint(sbi, &cpc);
 	}
 
 	/* be sure to wait for any on-going discard commands */
	f2fs_wait_discard_bios(sbi, true);
 
 	if (f2fs_discard_en(sbi) && !sbi->discard_blks) {
 		struct cp_control cpc = {
			.reason = CP_UMOUNT | CP_TRIMMED,
		};
		write_checkpoint(sbi, &cpc);
	}

	/* write_checkpoint can update stat informaion */
	f2fs_destroy_stats(sbi);

	/*
	 * normally superblock is clean, so we need to release this.
	 * In addition, EIO will skip do checkpoint, we need this as well.
	 */
	release_ino_entry(sbi, true);

	f2fs_leave_shrinker(sbi);
	mutex_unlock(&sbi->umount_mutex);

	/* our cp_error case, we can wait for any writeback page */
	f2fs_flush_merged_writes(sbi);

	iput(sbi->node_inode);
	iput(sbi->meta_inode);

	/* destroy f2fs internal modules */
	destroy_node_manager(sbi);
	destroy_segment_manager(sbi);

	kfree(sbi->ckpt);

	f2fs_unregister_sysfs(sbi);

	sb->s_fs_info = NULL;
	if (sbi->s_chksum_driver)
		crypto_free_shash(sbi->s_chksum_driver);
	kfree(sbi->raw_super);

	destroy_device_list(sbi);
	mempool_destroy(sbi->write_io_dummy);
#ifdef CONFIG_QUOTA
	for (i = 0; i < MAXQUOTAS; i++)
		kfree(sbi->s_qf_names[i]);
#endif
	destroy_percpu_info(sbi);
	for (i = 0; i < NR_PAGE_TYPE; i++)
		kfree(sbi->write_io[i]);
	kfree(sbi);
}