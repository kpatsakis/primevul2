void f2fs_wait_discard_bios(struct f2fs_sb_info *sbi)
void f2fs_wait_discard_bios(struct f2fs_sb_info *sbi, bool umount)
 {
 	__issue_discard_cmd(sbi, false);
 	__drop_discard_cmd(sbi);
	__wait_discard_cmd(sbi, !umount);
 }