callbacks_update_statusbar_coordinates (gint x, gint y)
{
	gdouble X, Y;

	callbacks_screen2board (&X, &Y, x, y);

	switch (screen.unit) {
	case GERBV_MILS:
		utf8_snprintf (screen.statusbar.coordstr, MAX_COORDLEN,
				_(gerbv_coords_pattern_mils_str),
				COORD2MILS (X), COORD2MILS (Y));
		break;
	case GERBV_MMS:
		utf8_snprintf (screen.statusbar.coordstr, MAX_COORDLEN,
				_("%8.3f %8.3f"),
				COORD2MMS (X), COORD2MMS (Y));
		break;
	default:
		utf8_snprintf (screen.statusbar.coordstr, MAX_COORDLEN,
				_("%4.5f %4.5f"),
				COORD2INS (X), COORD2INS (Y));
	}

	callbacks_update_statusbar();
}