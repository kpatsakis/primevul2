RList *r_bin_wasm_get_globals (RBinWasmObj *bin) {
	RBinWasmSection *global = NULL;
	RList *globals = NULL;

	if (!bin || !bin->g_sections) {
		return NULL;
	}

	if (bin->g_globals) {
		return bin->g_globals;
	}

	if (!(globals = r_bin_wasm_get_sections_by_id (bin->g_sections,
						R_BIN_WASM_SECTION_GLOBAL))) {
		return r_list_new();
	}

	if (!(global = (RBinWasmSection*) r_list_first (globals))) {
		return r_list_new();
	}

	bin->g_globals = r_bin_wasm_get_global_entries (bin, global);

	return bin->g_globals;
}
