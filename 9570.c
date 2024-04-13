check_align_files_possibility (gerbv_selection_info_t *sel_info)
{
	gerbv_fileinfo_t **f = mainProject->file;
	GtkMenuItem **menu_items = (GtkMenuItem **) screen.win.curEditAlingItem;
	gerbv_selection_item_t si[2];
	int id[2] = {-1, -1};
	int i;

	/* If has two objects, then can do files aligning */
	if (selection_length (sel_info) == 2) {
		si[0] = selection_get_item_by_index(sel_info, 0);
		si[1] = selection_get_item_by_index(sel_info, 1);

		for (i = 0; i <= mainProject->last_loaded; i++) {
			if (f[i]->image == si[0].image)
				id[0] = i;
						
			if (f[i]->image == si[1].image)
				id[1] = i;
		}

		/* Can align if on different files */
		if (id[0]*id[1] >= 0 && id[0] != id[1]) {
			gchar *str;

/* TODO: add color boxes for layers as hint */

			/* Update align menu items */
			str = g_strdup_printf (_("#_%i %s  >  #%i %s"),
					id[0]+1, f[id[0]]->name,
					id[1]+1, f[id[1]]->name);
			gtk_menu_item_set_label (menu_items[0], str);
			g_free (str);

			str = g_strdup_printf (_("#_%i %s  >  #%i %s"),
					id[1]+1, f[id[1]]->name,
					id[0]+1, f[id[0]]->name);
			gtk_menu_item_set_label (menu_items[1], str);
			g_free (str);

			gtk_widget_set_sensitive (
				screen.win.curEditAlingMenuItem, TRUE);

			return TRUE;
		}
	}

	/* Can't align, disable align menu */
	gtk_widget_set_sensitive (screen.win.curEditAlingMenuItem, FALSE);
	gtk_menu_item_set_label (menu_items[0], "");
	gtk_menu_item_set_label (menu_items[1], "");

	return FALSE;
}