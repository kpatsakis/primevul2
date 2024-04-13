static void _parse_resource_directory(struct PE_(r_bin_pe_obj_t) *bin, Pe_image_resource_directory *dir, ut64 offDir, int type, int id, SdbHash *dirs) {
	int index = 0;
	ut32 totalRes = dir->NumberOfNamedEntries + dir->NumberOfIdEntries;
	ut64 rsrc_base = bin->resource_directory_offset;
	ut64 off;
	if (totalRes > R_PE_MAX_RESOURCES) {
		return;
	}
	for (index = 0; index < totalRes; index++) {
		Pe_image_resource_directory_entry entry;
		off = rsrc_base + offDir + sizeof(*dir) + index * sizeof(entry);
		char *key = sdb_fmt ("0x%08"PFMT64x, off);
		if (sdb_ht_find (dirs, key, NULL)) {
			break;
		}
		sdb_ht_insert (dirs, key, "1");
		if (off > bin->size || off + sizeof (entry) > bin->size) {
			break;
		}
		if (r_buf_read_at (bin->b, off, (ut8*)&entry, sizeof(entry)) < 1) {
			eprintf ("Warning: read resource entry\n");
			break;
		}
		if (entry.u2.s.DataIsDirectory) {
			Pe_image_resource_directory identEntry;
			off = rsrc_base + entry.u2.s.OffsetToDirectory;
			int len = r_buf_read_at (bin->b, off, (ut8*) &identEntry, sizeof (identEntry));
			if (len < 1 || len != sizeof (Pe_image_resource_directory)) {
				eprintf ("Warning: parsing resource directory\n");
			}
			_parse_resource_directory (bin, &identEntry,
				entry.u2.s.OffsetToDirectory, type, entry.u1.Id, dirs);
			continue;
		}

		Pe_image_resource_data_entry *data = R_NEW0 (Pe_image_resource_data_entry);
		if (!data) {
			break;
		}
		off = rsrc_base + entry.u2.OffsetToData;
		if (off > bin->size || off + sizeof (data) > bin->size) {
			free (data);
			break;
		}
		if (r_buf_read_at (bin->b, off, (ut8*)data, sizeof (*data)) != sizeof (*data)) {
			eprintf ("Warning: read (resource data entry)\n");
			free (data);
			break;
		}
		if (type == PE_RESOURCE_ENTRY_VERSION) {
			char key[64];
			int counter = 0;
			Sdb *sdb = sdb_new0 ();
			if (!sdb) {
				free (data);
				sdb_free (sdb);
				continue;
			}
			PE_DWord data_paddr = bin_pe_rva_to_paddr (bin, data->OffsetToData);
			if (!data_paddr) {
				bprintf ("Warning: bad RVA in resource data entry\n");
				free (data);
				sdb_free (sdb);
				continue;
			}
			PE_DWord cur_paddr = data_paddr;
			if ((cur_paddr & 0x3) != 0) {
				bprintf ("Warning: not aligned version info address\n");
				free (data);
				sdb_free (sdb);
				continue;
			}
			while (cur_paddr < (data_paddr + data->Size) && cur_paddr < bin->size) {
				PE_VS_VERSIONINFO* vs_VersionInfo = Pe_r_bin_pe_parse_version_info (bin, cur_paddr);
				if (vs_VersionInfo) {
					snprintf (key, 30, "VS_VERSIONINFO%d", counter++);
					sdb_ns_set (sdb, key, Pe_r_bin_store_resource_version_info (vs_VersionInfo));
				} else {
					break;
				}
				if (vs_VersionInfo->wLength < 1) {
					break;
				}
				cur_paddr += vs_VersionInfo->wLength;
				free_VS_VERSIONINFO (vs_VersionInfo);
				align32 (cur_paddr);
			}
			sdb_ns_set (bin->kv, "vs_version_info", sdb);
		}
		r_pe_resource *rs = R_NEW0 (r_pe_resource);
		if (!rs) {
			free (data);
			break;
		}
		rs->timestr = _time_stamp_to_str (dir->TimeDateStamp);
		rs->type = strdup (_resource_type_str (type));
		rs->language = strdup (_resource_lang_str (entry.u1.Name & 0x3ff));
		rs->data = data;
		rs->name = id;
		r_list_append (bin->resources, rs);
	}
}
