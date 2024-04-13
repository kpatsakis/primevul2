callbacks_generic_save_activate (GtkMenuItem     *menuitem,
				 gpointer         user_data)
{
	gchar *new_file_name = NULL;
	gchar *file_name = NULL;
	gchar *dir_name = NULL;
	gboolean error_visible_layers = FALSE;
	gchar *windowTitle = NULL;
	gerbv_fileinfo_t *act_file;
	gint file_index;
	gint processType = GPOINTER_TO_INT (user_data);
	GtkFileFilter *filter;
	GtkSpinButton *spin_but;
	GtkTooltips *tooltips;
	GtkWidget *label;
	GtkWidget *hbox;
	static gint dpi = 0;
	
	file_index = callbacks_get_selected_row_index ();
	if (file_index < 0) {
		interface_show_alert_dialog (_("No layer is currently active"),
			_("Please select a layer and try again."),
			FALSE,
			NULL);
		return;
	}

	act_file = mainProject->file[file_index];

	screen.win.gerber = gtk_file_chooser_dialog_new ("", NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE, NULL, NULL, NULL);
	GtkFileChooser *file_chooser_p =
			GTK_FILE_CHOOSER(screen.win.gerber);
	gtk_file_chooser_set_do_overwrite_confirmation (file_chooser_p, TRUE);

	hbox = gtk_hbox_new (0, 0);
	spin_but = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0, 0, 1));
	label = gtk_label_new ("");
	tooltips = gtk_tooltips_new ();
	gtk_box_pack_end (GTK_BOX(hbox), GTK_WIDGET(spin_but), 0, 0, 1);
	gtk_box_pack_end (GTK_BOX(hbox), label, 0, 0, 5);
	gtk_box_pack_end (GTK_BOX(GTK_DIALOG(screen.win.gerber)->vbox),
			hbox, 0, 0, 2);

	switch (processType) {
	case CALLBACKS_SAVE_PROJECT_AS:
		windowTitle = g_strdup (_("Save project as..."));
		if (mainProject->project) {
			file_name = g_path_get_basename (mainProject->project);

			dir_name = g_path_get_dirname (mainProject->project);
		} else {
			file_name = g_strdup_printf("%s%s",
					pgettext("file name", "untitled"),
					GERBV_PROJECT_FILE_EXT);
			dir_name= g_path_get_dirname (act_file->fullPathname);
		}

		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, _(gerbv_project_file_name));
		gtk_file_filter_add_pattern (filter, gerbv_project_file_pat);
		gtk_file_chooser_add_filter (file_chooser_p, filter);

		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, _("All"));
		gtk_file_filter_add_pattern (filter, "*");
		gtk_file_chooser_add_filter (file_chooser_p, filter);

		break;
	case CALLBACKS_SAVE_FILE_PS:
		windowTitle = g_strdup_printf (
				_("Export visible layers to %s file as..."),
				_("PS"));
		if (0 == visible_file_name(&file_name, &dir_name, -1,
							".ps", ".ps")) {
			error_visible_layers = TRUE;
			break;
		}


		break;
	case CALLBACKS_SAVE_FILE_PDF:
		windowTitle = g_strdup_printf (
				_("Export visible layers to %s file as..."),
				_("PDF"));
		if (0 == visible_file_name(&file_name, &dir_name, -1,
							".pdf", ".pdf")) {
			error_visible_layers = TRUE;
			break;
		}


		break;
	case CALLBACKS_SAVE_FILE_SVG:
		windowTitle = g_strdup_printf (
				_("Export visible layers to %s file as..."),
				_("SVG"));
		if (0 == visible_file_name(&file_name, &dir_name, -1,
							".svg", ".svg")) {
			error_visible_layers = TRUE;
			break;
		}


		break;
	case CALLBACKS_SAVE_FILE_DXF:
#if HAVE_LIBDXFLIB
		windowTitle = g_strdup_printf (
			_("Export \"%s\" layer #%d to DXF file as..."),
			act_file->name, file_index + 1);
		file_name = g_strconcat (act_file->name, ".dxf", NULL);
		dir_name =  g_path_get_dirname (act_file->fullPathname);
#endif
		break;
	case CALLBACKS_SAVE_FILE_PNG:
		windowTitle = g_strdup_printf (
				_("Export visible layers to %s file as..."),
				_("PNG"));
		if (0 == visible_file_name(&file_name, &dir_name, -1,
							".png", ".png")) {
			error_visible_layers = TRUE;
			break;
		}

		gtk_label_set_text (GTK_LABEL(label), _("DPI:"));
		gtk_spin_button_set_range (spin_but, 0, 6000);
		gtk_spin_button_set_increments (spin_but, 10, 100);
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET(label),
				_("DPI value, autoscaling if 0"), NULL);
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET(spin_but),
				_("DPI value, autoscaling if 0"), NULL);
		gtk_spin_button_set_value (spin_but, dpi);
		gtk_widget_show_all (hbox);

		break;
	case CALLBACKS_SAVE_FILE_RS274X:
		windowTitle = g_strdup_printf(
			_("Export \"%s\" layer #%d to "
				"RS-274X file as..."),
			act_file->name, file_index+1);

		if (GERBV_LAYERTYPE_RS274X != act_file->image->layertype)
			file_name = g_strconcat (act_file->name, ".gbr", NULL);
		else
			file_name = g_strdup (act_file->name);

		dir_name =  g_path_get_dirname (act_file->fullPathname);
		break;
	case CALLBACKS_SAVE_FILE_RS274XM:
		windowTitle = g_strdup (_("Export merged visible layers to "
					"RS-274X file as..."));
		if (2 > visible_file_name(&file_name, &dir_name,
					GERBV_LAYERTYPE_RS274X, "", ".gbr")) {
			error_visible_layers = TRUE;
			break;
		}
	case CALLBACKS_SAVE_FILE_DRILL:
		windowTitle = g_strdup_printf(
			_("Export \"%s\" layer #%d to "
				"Excellon drill file as..."),
			act_file->name, file_index+1);

		if (GERBV_LAYERTYPE_DRILL != act_file->image->layertype)
			file_name = g_strconcat (act_file->name, ".drl", NULL);
		else
			file_name = g_strdup (act_file->name);

		dir_name =  g_path_get_dirname (act_file->fullPathname);
		break;
	case CALLBACKS_SAVE_FILE_DRILLM:
		windowTitle = g_strdup (_("Export merged visible layers to "
					"Excellon drill file as..."));
		if (2 > visible_file_name(&file_name, &dir_name,
				GERBV_LAYERTYPE_DRILL, "", ".drl")) {
			error_visible_layers = TRUE;
		}
		break;
	case CALLBACKS_SAVE_FILE_IDRILL:
		windowTitle = g_strdup_printf(
			_("Export \"%s\" layer #%d to ISEL NCP drill file as..."),
			act_file->name, file_index+1);
		file_name = g_strconcat (act_file->name, ".ncp", NULL);
		dir_name =  g_path_get_dirname (act_file->fullPathname);

		break;
	case CALLBACKS_SAVE_FILE_GEDA_PCB:
		windowTitle = g_strdup_printf (
			_("Export \"%s\" layer #%d to gEDA PCB file as..."),
			act_file->name, file_index + 1);
		file_name = g_strconcat (act_file->name, ".pcb", NULL);
		dir_name =  g_path_get_dirname (act_file->fullPathname);
		break;
	case CALLBACKS_SAVE_LAYER_AS:
		windowTitle = g_strdup_printf (_("Save \"%s\" layer #%d as..."),
				act_file->name, file_index+1);
		file_name = g_strdup (act_file->name);
		dir_name =  g_path_get_dirname (act_file->fullPathname);
		break;
	}

	if (file_name != NULL) {
		gtk_file_chooser_set_current_name (file_chooser_p, file_name);
		g_free (file_name);
	}
	if (dir_name != NULL) {
		gtk_file_chooser_set_current_folder (file_chooser_p, dir_name);
		g_free (dir_name);
	}

	gtk_dialog_add_buttons (GTK_DIALOG(screen.win.gerber),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE,   GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_window_set_title (GTK_WINDOW(screen.win.gerber), windowTitle);
	g_free (windowTitle);

	if (error_visible_layers) {
		switch (processType) {
		case CALLBACKS_SAVE_FILE_RS274XM:
			interface_get_alert_dialog_response (
				_("Not enough Gerber layers are visible"),
				_("Two or more Gerber layers must be visible "
				"for export with merge."),
				FALSE, NULL, NULL, GTK_STOCK_CANCEL);
			break;
		case CALLBACKS_SAVE_FILE_DRILLM:
			interface_get_alert_dialog_response (
				_("Not enough Excellon layers are visible"),
				_("Two or more Excellon layers must be visible "
				"for export with merge."),
				FALSE, NULL, NULL, GTK_STOCK_CANCEL);
			break;
		default:
			interface_get_alert_dialog_response (
				_("No layers are visible"), _("One or more "
				"layers must be visible for export."),
				FALSE, NULL, NULL, GTK_STOCK_CANCEL);
		}

		gtk_widget_destroy (screen.win.gerber);
		callbacks_update_layer_tree ();

		return;
	}

	gtk_widget_show (screen.win.gerber);
	if (gtk_dialog_run (GTK_DIALOG(screen.win.gerber)) == GTK_RESPONSE_ACCEPT) {
		new_file_name = gtk_file_chooser_get_filename (file_chooser_p);
		dpi = gtk_spin_button_get_value_as_int (spin_but);
	}
	gtk_widget_destroy (screen.win.gerber);

	if (!new_file_name) {
		callbacks_update_layer_tree ();

		return;
	}

	switch (processType) {
	case CALLBACKS_SAVE_PROJECT_AS:
		main_save_as_project_from_filename (mainProject, new_file_name);
		rename_main_window(new_file_name, NULL);
		break;
	case CALLBACKS_SAVE_FILE_PS:
		gerbv_export_postscript_file_from_project_autoscaled (
				mainProject, new_file_name);
		break;
	case CALLBACKS_SAVE_FILE_PDF:
		gerbv_export_pdf_file_from_project_autoscaled (
				mainProject, new_file_name);
		break;
	case CALLBACKS_SAVE_FILE_SVG:
		gerbv_export_svg_file_from_project_autoscaled (
				mainProject, new_file_name);
		break;
	case CALLBACKS_SAVE_FILE_DXF:
#if HAVE_LIBDXFLIB
		if (gerbv_export_dxf_file_from_image(new_file_name,
				act_file->image, &act_file->transform)) {
			GERB_MESSAGE (
				_("\"%s\" layer #%d saved as DXF in \"%s\""),
				act_file->name, file_index + 1,
				new_file_name);
		}
#endif
		break;
	case CALLBACKS_SAVE_FILE_PNG:
		if (dpi == 0) {
			gerbv_export_png_file_from_project_autoscaled (
					mainProject,
					screenRenderInfo.displayWidth,
					screenRenderInfo.displayHeight,
					new_file_name);
		} else {	/* Non zero DPI */
			gerbv_render_size_t bb;
			gerbv_render_get_boundingbox (mainProject, &bb);
			gfloat w = bb.right - bb.left;
			gfloat h = bb.bottom - bb.top;
			gerbv_render_info_t renderInfo = {
				dpi, dpi,
				bb.left - (w*GERBV_DEFAULT_BORDER_COEFF)/2.0,
				bb.top - (h*GERBV_DEFAULT_BORDER_COEFF)/2.0,
				GERBV_RENDER_TYPE_CAIRO_HIGH_QUALITY,
				w*dpi*(1 + GERBV_DEFAULT_BORDER_COEFF),
				h*dpi*(1 + GERBV_DEFAULT_BORDER_COEFF),
			};
			gerbv_export_png_file_from_project (mainProject,
					&renderInfo, new_file_name);
		}

		break;
	case  CALLBACKS_SAVE_LAYER_AS:
		gerbv_save_layer_from_index (mainProject,
				file_index, new_file_name);

		/* Rename the file path in the index, so future saves will
		 * reference the new file path */
		g_free (act_file->fullPathname);
		act_file->fullPathname = g_strdup (new_file_name);
		g_free (act_file->name);
		act_file->name = g_path_get_basename (new_file_name);

		break;
	case CALLBACKS_SAVE_FILE_RS274X:
		if (gerbv_export_rs274x_file_from_image (new_file_name,
					act_file->image,
					&act_file->transform)) {
			GERB_MESSAGE (
				_("\"%s\" layer #%d saved as Gerber in \"%s\""),
				act_file->name, file_index + 1,
				new_file_name);
		}
		break;
	case CALLBACKS_SAVE_FILE_DRILL:
		if (gerbv_export_drill_file_from_image (new_file_name,
					act_file->image,
					&act_file->transform)) {
			GERB_MESSAGE (
				_("\"%s\" layer #%d saved as drill in \"%s\""),
				act_file->name, file_index + 1,
				new_file_name);
		}
		break;
	case CALLBACKS_SAVE_FILE_IDRILL:
		if (gerbv_export_isel_drill_file_from_image (new_file_name,
				act_file->image, &act_file->transform)) {
			GERB_MESSAGE (
				_("\"%s\" layer #%d saved as ISEL NCP drill "
				"in \"%s\""), act_file->name, file_index + 1,
				new_file_name);
		}
		break;
	case CALLBACKS_SAVE_FILE_GEDA_PCB:
		if (gerbv_export_geda_pcb_file_from_image(new_file_name,
				act_file->image, &act_file->transform)) {
			GERB_MESSAGE (
				_("\"%s\" layer #%d saved as gEDA PCB "
				"in \"%s\""), act_file->name, file_index + 1,
				new_file_name);
		}
		break;
	case CALLBACKS_SAVE_FILE_RS274XM: {
		gerbv_image_t *image;
		gerbv_user_transformation_t t = {0,0,1,1,0,FALSE,FALSE,FALSE};
		if (NULL != (image = merge_images (processType))) {
			if (gerbv_export_rs274x_file_from_image (
						new_file_name, image, &t)) {
				GERB_MESSAGE (_("Merged visible Gerber layers "
						"and saved in \"%s\""),
						new_file_name);
			}
			gerbv_destroy_image (image);
		}
		break;
	}
	case CALLBACKS_SAVE_FILE_DRILLM: {
		gerbv_image_t *image;
		gerbv_user_transformation_t t = {0,0,1,1,0,FALSE,FALSE,FALSE};
		if (NULL != (image = merge_images (processType))) {
			gerbv_export_drill_file_from_image (
					new_file_name, image, &t);
			gerbv_destroy_image (image);
			GERB_MESSAGE (_("Merged visible drill layers "
					"and saved in \"%s\""),
					new_file_name);
		}
		break;
	}
	}

	g_free (new_file_name);
	callbacks_update_layer_tree ();

	return;
}