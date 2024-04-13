 static int nfc_llcp_build_gb(struct nfc_llcp_local *local)
 {
	u8 *gb_cur, version, version_length;
	u8 lto_length, wks_length, miux_length;
	u8 *version_tlv = NULL, *lto_tlv = NULL,
	   *wks_tlv = NULL, *miux_tlv = NULL;
 	__be16 wks = cpu_to_be16(local->local_wks);
 	u8 gb_len = 0;
 	int ret = 0;
 
 	version = LLCP_VERSION_11;
 	version_tlv = nfc_llcp_build_tlv(LLCP_TLV_VERSION, &version,
 					 1, &version_length);
	if (!version_tlv) {
		ret = -ENOMEM;
		goto out;
	}
 	gb_len += version_length;
 
 	lto_tlv = nfc_llcp_build_tlv(LLCP_TLV_LTO, &local->lto, 1, &lto_length);
	if (!lto_tlv) {
		ret = -ENOMEM;
		goto out;
	}
 	gb_len += lto_length;
 
 	pr_debug("Local wks 0x%lx\n", local->local_wks);
 	wks_tlv = nfc_llcp_build_tlv(LLCP_TLV_WKS, (u8 *)&wks, 2, &wks_length);
	if (!wks_tlv) {
		ret = -ENOMEM;
		goto out;
	}
 	gb_len += wks_length;
 
 	miux_tlv = nfc_llcp_build_tlv(LLCP_TLV_MIUX, (u8 *)&local->miux, 0,
 				      &miux_length);
	if (!miux_tlv) {
		ret = -ENOMEM;
		goto out;
	}
 	gb_len += miux_length;
 
 	gb_len += ARRAY_SIZE(llcp_magic);

	if (gb_len > NFC_MAX_GT_LEN) {
		ret = -EINVAL;
		goto out;
	}

	gb_cur = local->gb;

	memcpy(gb_cur, llcp_magic, ARRAY_SIZE(llcp_magic));
	gb_cur += ARRAY_SIZE(llcp_magic);

	memcpy(gb_cur, version_tlv, version_length);
	gb_cur += version_length;

	memcpy(gb_cur, lto_tlv, lto_length);
	gb_cur += lto_length;

	memcpy(gb_cur, wks_tlv, wks_length);
	gb_cur += wks_length;

	memcpy(gb_cur, miux_tlv, miux_length);
	gb_cur += miux_length;

	local->gb_len = gb_len;

out:
	kfree(version_tlv);
	kfree(lto_tlv);
	kfree(wks_tlv);
	kfree(miux_tlv);

	return ret;
}
