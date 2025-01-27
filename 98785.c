setup_boot_parameters(struct kimage *image, struct boot_params *params,
		      unsigned long params_load_addr,
		      unsigned int efi_map_offset, unsigned int efi_map_sz,
		      unsigned int efi_setup_data_offset)
{
	unsigned int nr_e820_entries;
	unsigned long long mem_k, start, end;
	int i, ret = 0;

	/* Get subarch from existing bootparams */
	params->hdr.hardware_subarch = boot_params.hdr.hardware_subarch;

	/* Copying screen_info will do? */
	memcpy(&params->screen_info, &boot_params.screen_info,
				sizeof(struct screen_info));

	/* Fill in memsize later */
	params->screen_info.ext_mem_k = 0;
	params->alt_mem_k = 0;

	/* Default APM info */
	memset(&params->apm_bios_info, 0, sizeof(params->apm_bios_info));

	/* Default drive info */
	memset(&params->hd0_info, 0, sizeof(params->hd0_info));
	memset(&params->hd1_info, 0, sizeof(params->hd1_info));

	if (image->type == KEXEC_TYPE_CRASH) {
		ret = crash_setup_memmap_entries(image, params);
		if (ret)
			return ret;
	} else
		setup_e820_entries(params);

	nr_e820_entries = params->e820_entries;

	for (i = 0; i < nr_e820_entries; i++) {
		if (params->e820_map[i].type != E820_RAM)
			continue;
		start = params->e820_map[i].addr;
		end = params->e820_map[i].addr + params->e820_map[i].size - 1;

		if ((start <= 0x100000) && end > 0x100000) {
			mem_k = (end >> 10) - (0x100000 >> 10);
			params->screen_info.ext_mem_k = mem_k;
			params->alt_mem_k = mem_k;
			if (mem_k > 0xfc00)
				params->screen_info.ext_mem_k = 0xfc00; /* 64M*/
			if (mem_k > 0xffffffff)
				params->alt_mem_k = 0xffffffff;
		}
	}

#ifdef CONFIG_EFI
	/* Setup EFI state */
	setup_efi_state(params, params_load_addr, efi_map_offset, efi_map_sz,
			efi_setup_data_offset);
#endif

	/* Setup EDD info */
	memcpy(params->eddbuf, boot_params.eddbuf,
				EDDMAXNR * sizeof(struct edd_info));
	params->eddbuf_entries = boot_params.eddbuf_entries;

	memcpy(params->edd_mbr_sig_buffer, boot_params.edd_mbr_sig_buffer,
	       EDD_MBR_SIG_MAX * sizeof(unsigned int));

	return ret;
}
