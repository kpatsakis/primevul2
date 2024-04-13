void _CbInRangeAav(RCore *core, ut64 from, ut64 to, int vsize, bool asterisk, int count) {
	bool isarm = archIsArmOrThumb (core);
	if (isarm) {
		if (to & 1) {
			to--;
			r_anal_hint_set_bits (core->anal, to, 16);
		} else {
			r_core_seek_archbits (core, from);
			ut64 bits = r_config_get_i (core->config, "asm.bits");
			r_anal_hint_set_bits (core->anal, from, bits);
		}
	} else {
		bool ismips = archIsMips (core);
		if (ismips) {
			if (from % 4 || to % 4) {
				eprintf ("False positive\n");
				return;
			}
		}
	}
	if (asterisk) {
		r_cons_printf ("ax 0x%"PFMT64x " 0x%"PFMT64x "\n", to, from);
		r_cons_printf ("Cd %d @ 0x%"PFMT64x "\n", vsize, from);
		r_cons_printf ("f+ aav.0x%08"PFMT64x "= 0x%08"PFMT64x, to, to);
	} else {
#if 1
		r_anal_ref_add (core->anal, to, from, ' ');
		r_meta_add (core->anal, 'd', from, from + vsize, NULL);
		if (!r_flag_get_at (core->flags, to, false)) {
			char *name = r_str_newf ("aav.0x%08"PFMT64x, to);
			r_flag_set (core->flags, name, to, vsize);
			free (name);
		}
#else
		r_core_cmdf (core, "ax 0x%"PFMT64x " 0x%"PFMT64x, to, from);
		r_core_cmdf (core, "Cd %d @ 0x%"PFMT64x, vsize, from);
		r_core_cmdf (core, "f+ aav.0x%08"PFMT64x "= 0x%08"PFMT64x, to, to);
#endif
	}
}
