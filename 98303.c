RList *r_bin_wasm_get_elements (RBinWasmObj *bin) {
	RBinWasmSection *element = NULL;
	RList *elements = NULL;

	if (!bin || !bin->g_sections) {
		return NULL;
	}

	if (bin->g_elements) {
		return bin->g_elements;
	}

	if (!(elements = r_bin_wasm_get_sections_by_id (bin->g_sections,
						R_BIN_WASM_SECTION_ELEMENT))) {
		return r_list_new();
	}

	if (!(element = (RBinWasmSection*) r_list_first (elements))) {
		return r_list_new();
	}

	bin->g_elements = r_bin_wasm_get_element_entries (bin, element);

	return bin->g_elements;
}
