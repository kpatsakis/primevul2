static int bin_pe_init_imports(struct PE_(r_bin_pe_obj_t)* bin) {
	PE_(image_data_directory) * data_dir_import = &bin->data_directory[PE_IMAGE_DIRECTORY_ENTRY_IMPORT];
	PE_(image_data_directory) * data_dir_delay_import = &bin->data_directory[PE_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];

	PE_DWord import_dir_paddr = bin_pe_rva_to_paddr (bin, data_dir_import->VirtualAddress);
	PE_DWord import_dir_offset = bin_pe_rva_to_paddr (bin, data_dir_import->VirtualAddress);
	PE_DWord delay_import_dir_offset = data_dir_delay_import
	? bin_pe_rva_to_paddr (bin, data_dir_delay_import->VirtualAddress)
					: 0;

	PE_(image_import_directory) * import_dir = NULL;
	PE_(image_import_directory) * new_import_dir = NULL;
	PE_(image_import_directory) * curr_import_dir = NULL;

	PE_(image_delay_import_directory) * delay_import_dir = NULL;
	PE_(image_delay_import_directory) * curr_delay_import_dir = NULL;

	int dir_size = sizeof(PE_(image_import_directory));
	int delay_import_size = sizeof(PE_(image_delay_import_directory));
	int indx = 0;
	int rr, count = 0;
	int import_dir_size = data_dir_import->Size;
	int delay_import_dir_size = data_dir_delay_import->Size;
	if (!import_dir_size) {
		import_dir_size = data_dir_import->Size = 0xffff;
	}
	if (!delay_import_dir_size) {
		delay_import_dir_size = data_dir_delay_import->Size = 0xffff;
	}
	int maxidsz = R_MIN ((PE_DWord) bin->size, import_dir_offset + import_dir_size);
	maxidsz -= import_dir_offset;
	if (maxidsz < 0) {
		maxidsz = 0;
	}

	free (bin->import_directory);
	bin->import_directory = NULL;
	if (import_dir_paddr != 0) {
		if (import_dir_size < 1 || import_dir_size > maxidsz) {
			bprintf ("Warning: Invalid import directory size: 0x%x is now 0x%x\n", import_dir_size, maxidsz);
			import_dir_size = maxidsz;
		}
		bin->import_directory_offset = import_dir_offset;
		count = 0;
		do {
			indx++;
			if (((2 + indx) * dir_size) > import_dir_size) {
				break; //goto fail;
			}
			new_import_dir = (PE_(image_import_directory)*)realloc (import_dir, ((1 + indx) * dir_size));
			if (!new_import_dir) {
				r_sys_perror ("malloc (import directory)");
				free (import_dir);
				import_dir = NULL;
				break; //
			}
			import_dir = new_import_dir;
			new_import_dir = NULL;
			curr_import_dir = import_dir + (indx - 1);
			if (r_buf_read_at (bin->b, import_dir_offset + (indx - 1) * dir_size, (ut8*) (curr_import_dir), dir_size) < 1) {
				bprintf ("Warning: read (import directory)\n");
				free (import_dir);
				import_dir = NULL;
				break; //return false;
			}
			count++;
		} while (curr_import_dir->FirstThunk != 0 || curr_import_dir->Name != 0 ||
		curr_import_dir->TimeDateStamp != 0 || curr_import_dir->Characteristics != 0 ||
		curr_import_dir->ForwarderChain != 0);

		bin->import_directory = import_dir;
		bin->import_directory_size = import_dir_size;
	}

	indx = 0;
	if (bin->b->length > 0) {
		if ((delay_import_dir_offset != 0) && (delay_import_dir_offset < (ut32) bin->b->length)) {
			ut64 off;
			bin->delay_import_directory_offset = delay_import_dir_offset;
			do {
				indx++;
				off = indx * delay_import_size;
				if (off >= bin->b->length) {
					bprintf ("Warning: Cannot find end of import symbols\n");
					break;
				}
				delay_import_dir = (PE_(image_delay_import_directory)*)realloc (
					delay_import_dir, (indx * delay_import_size) + 1);
				if (delay_import_dir == 0) {
					r_sys_perror ("malloc (delay import directory)");
					free (delay_import_dir);
					return false;
				}

				curr_delay_import_dir = delay_import_dir + (indx - 1);
				rr = r_buf_read_at (bin->b, delay_import_dir_offset + (indx - 1) * delay_import_size,
					(ut8*) (curr_delay_import_dir), dir_size);
				if (rr != dir_size) {
					bprintf ("Warning: read (delay import directory)\n");
					goto fail;
				}
			} while (curr_delay_import_dir->Name != 0);
			bin->delay_import_directory = delay_import_dir;
		}
	}

	return true;
fail:
	free (import_dir);
	import_dir = NULL;
	bin->import_directory = import_dir;
	free (delay_import_dir);
	return false;
}
