static int __init fm10k_init_module(void)
{
	pr_info("%s - version %s\n", fm10k_driver_string, fm10k_driver_version);
	pr_info("%s\n", fm10k_copyright);

 	/* create driver workqueue */
 	fm10k_workqueue = alloc_workqueue("%s", WQ_MEM_RECLAIM, 0,
 					  fm10k_driver_name);
	if (!fm10k_workqueue)
		return -ENOMEM;
 
 	fm10k_dbg_init();
 
	return fm10k_register_pci_driver();
}
