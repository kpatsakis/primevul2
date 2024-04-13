static int bin_symbols(RCore *r, int mode, ut64 laddr, int va, ut64 at, const char *name, bool exponly, const char *args) {
	RBinInfo *info = r_bin_get_info (r->bin);
	RList *entries = r_bin_get_entries (r->bin);
	RBinSymbol *symbol;
	RBinAddr *entry;
	RListIter *iter;
	bool firstexp = true;
	bool printHere = false;
	int i = 0, lastfs = 's';
	bool bin_demangle = r_config_get_i (r->config, "bin.demangle");
	if (!info) {
		return 0;
	}

	if (args && *args == '.') {
		printHere = true;
	}

	bool is_arm = info && info->arch && !strncmp (info->arch, "arm", 3);
	const char *lang = bin_demangle ? r_config_get (r->config, "bin.lang") : NULL;

	RList *symbols = r_bin_get_symbols (r->bin);
	r_spaces_push (&r->anal->meta_spaces, "bin");

	if (IS_MODE_JSON (mode) && !printHere) {
		r_cons_printf ("[");
	} else if (IS_MODE_SET (mode)) {
		r_flag_space_set (r->flags, R_FLAGS_FS_SYMBOLS);
	} else if (!at && exponly) {
		if (IS_MODE_RAD (mode)) {
			r_cons_printf ("fs exports\n");
		} else if (IS_MODE_NORMAL (mode)) {
			r_cons_printf (printHere ? "" : "[Exports]\n");
		}
	} else if (!at && !exponly) {
		if (IS_MODE_RAD (mode)) {
			r_cons_printf ("fs symbols\n");
		} else if (IS_MODE_NORMAL (mode)) {
			r_cons_printf (printHere ? "" : "[Symbols]\n");
		}
	}
	if (IS_MODE_NORMAL (mode)) {
		r_cons_printf ("Num Paddr      Vaddr      Bind     Type Size Name\n");
	}


	size_t count = 0;
	r_list_foreach (symbols, iter, symbol) {
		if (!symbol->name) {
			continue;
		}
		char *r_symbol_name = r_str_escape_utf8 (symbol->name, false, true);
		ut64 addr = compute_addr (r->bin, symbol->paddr, symbol->vaddr, va);
		int len = symbol->size ? symbol->size : 32;
		SymName sn = {0};

		if (exponly && !isAnExport (symbol)) {
			free (r_symbol_name);
			continue;
		}
		if (name && strcmp (r_symbol_name, name)) {
			free (r_symbol_name);
			continue;
		}
		if (at && (!symbol->size || !is_in_range (at, addr, symbol->size))) {
			free (r_symbol_name);
			continue;
		}
		if ((printHere && !is_in_range (r->offset, symbol->paddr, len))
				&& (printHere && !is_in_range (r->offset, addr, len))) {
			free (r_symbol_name);
			continue;
		}
		count ++;
		snInit (r, &sn, symbol, lang);

		if (IS_MODE_SET (mode) && (is_section_symbol (symbol) || is_file_symbol (symbol))) {
			/*
			 * Skip section symbols because they will have their own flag.
			 * Skip also file symbols because not useful for now.
			 */
		} else if (IS_MODE_SET (mode) && is_special_symbol (symbol)) {
			if (is_arm) {
				handle_arm_special_symbol (r, symbol, va);
			}
		} else if (IS_MODE_SET (mode)) {
			if (is_arm) {
				handle_arm_symbol (r, symbol, info, va);
			}
			select_flag_space (r, symbol);
			/* If that's a Classed symbol (method or so) */
			if (sn.classname) {
				RFlagItem *fi = r_flag_get (r->flags, sn.methflag);
				if (r->bin->prefix) {
					char *prname = r_str_newf ("%s.%s", r->bin->prefix, sn.methflag);
					r_name_filter (sn.methflag, -1);
					free (sn.methflag);
					sn.methflag = prname;
				}
				if (fi) {
					r_flag_item_set_realname (fi, sn.methname);
					if ((fi->offset - r->flags->base) == addr) {
						r_flag_unset (r->flags, fi);
					}
				} else {
					fi = r_flag_set (r->flags, sn.methflag, addr, symbol->size);
					char *comment = fi->comment ? strdup (fi->comment) : NULL;
					if (comment) {
						r_flag_item_set_comment (fi, comment);
						R_FREE (comment);
					}
				}
			} else {
				const char *n = sn.demname ? sn.demname : sn.name;
				const char *fn = sn.demflag ? sn.demflag : sn.nameflag;
				char *fnp = (r->bin->prefix) ?
					r_str_newf ("%s.%s", r->bin->prefix, fn):
					strdup (fn);
				RFlagItem *fi = r_flag_set (r->flags, fnp, addr, symbol->size);
				if (fi) {
					r_flag_item_set_realname (fi, n);
					fi->demangled = (bool)(size_t)sn.demname;
				} else {
					if (fn) {
						eprintf ("[Warning] Can't find flag (%s)\n", fn);
					}
				}
				free (fnp);
			}
			if (sn.demname) {
				r_meta_add (r->anal, R_META_TYPE_COMMENT,
					addr, symbol->size, sn.demname);
			}
			r_flag_space_pop (r->flags);
		} else if (IS_MODE_JSON (mode)) {
			char *str = r_str_escape_utf8_for_json (r_symbol_name, -1);
			r_cons_printf ("%s{\"name\":\"%s\","
				"\"demname\":\"%s\","
				"\"flagname\":\"%s\","
				"\"ordinal\":%d,"
				"\"bind\":\"%s\","
				"\"size\":%d,"
				"\"type\":\"%s\","
				"\"vaddr\":%"PFMT64d","
				"\"paddr\":%"PFMT64d"}",
				((exponly && firstexp) || printHere) ? "" : (iter->p ? "," : ""),
				str,
				sn.demname? sn.demname: "",
				sn.nameflag,
				symbol->ordinal,
				symbol->bind,
				(int)symbol->size,
				symbol->type,
				(ut64)addr, (ut64)symbol->paddr);
			free (str);
		} else if (IS_MODE_SIMPLE (mode)) {
			const char *name = sn.demname? sn.demname: r_symbol_name;
			r_cons_printf ("0x%08"PFMT64x" %d %s\n",
				addr, (int)symbol->size, name);
		} else if (IS_MODE_SIMPLEST (mode)) {
			const char *name = sn.demname? sn.demname: r_symbol_name;
			r_cons_printf ("%s\n", name);
		} else if (IS_MODE_RAD (mode)) {
			/* Skip special symbols because we do not flag them and
			 * they shouldn't be printed in the rad format either */
			if (is_special_symbol (symbol)) {
				goto next;
			}
			RBinFile *binfile;
			RBinPlugin *plugin;
			const char *name = sn.demname? sn.demname: r_symbol_name;
			if (!name) {
				goto next;
			}
			if (!strncmp (name, "imp.", 4)) {
				if (lastfs != 'i') {
					r_cons_printf ("fs imports\n");
				}
				lastfs = 'i';
			} else {
				if (lastfs != 's') {
					const char *fs = exponly? "exports": "symbols";
					r_cons_printf ("fs %s\n", fs);
				}
				lastfs = 's';
			}
			if (r->bin->prefix || *name) { // we don't want unnamed symbol flags
				char *flagname = construct_symbol_flagname ("sym", name, MAXFLAG_LEN_DEFAULT);
				if (!flagname) {
					goto next;
				}
				r_cons_printf ("\"f %s%s%s %u 0x%08" PFMT64x "\"\n",
					r->bin->prefix ? r->bin->prefix : "", r->bin->prefix ? "." : "",
					flagname, symbol->size, addr);
				free (flagname);
			}
			binfile = r_bin_cur (r->bin);
			plugin = r_bin_file_cur_plugin (binfile);
			if (plugin && plugin->name) {
				if (r_str_startswith (plugin->name, "pe")) {
					char *module = strdup (r_symbol_name);
					char *p = strstr (module, ".dll_");
					if (p && strstr (module, "imp.")) {
						char *symname = __filterShell (p + 5);
 						char *m = __filterShell (module);
 						*p = 0;
 						if (r->bin->prefix) {
							r_cons_printf ("\"k bin/pe/%s/%d=%s.%s\"\n",
 								module, symbol->ordinal, r->bin->prefix, symname);
 						} else {
							r_cons_printf ("\"k bin/pe/%s/%d=%s\"\n",
 								module, symbol->ordinal, symname);
 						}
 						free (symname);
						free (m);
					}
					free (module);
				}
			}
		} else {
			const char *bind = symbol->bind? symbol->bind: "NONE";
			const char *type = symbol->type? symbol->type: "NONE";
			const char *name = r_str_get (sn.demname? sn.demname: r_symbol_name);
			r_cons_printf ("%03u", symbol->ordinal);
			if (symbol->paddr == UT64_MAX) {
				r_cons_printf (" ----------");
			} else {
				r_cons_printf (" 0x%08"PFMT64x, symbol->paddr);
			}
			r_cons_printf (" 0x%08"PFMT64x" %6s %6s %4d%s%s\n",
			               addr, bind, type, symbol->size, *name? " ": "", name);
		}
next:
		snFini (&sn);
		i++;
		free (r_symbol_name);
		if (exponly && firstexp) {
			firstexp = false;
		}
		if (printHere) {
			break;
		}
	}
	if (count == 0 && IS_MODE_JSON (mode)) {
		r_cons_printf ("{}");
	}


	if (is_arm) {
		r_list_foreach (entries, iter, entry) {
			if (IS_MODE_SET (mode)) {
				handle_arm_entry (r, entry, info, va);
			}
		}
	}
	if (IS_MODE_JSON (mode) && !printHere) {
		r_cons_printf ("]");
	}

	r_spaces_pop (&r->anal->meta_spaces);
	return true;
}
