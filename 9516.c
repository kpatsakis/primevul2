callbacks_analyze_active_gerbers_activate(GtkMenuItem *menuitem,
					gpointer user_data)
{
	gerbv_aperture_list_t *aperture_list;
	gchar *str;
	int i;

	/* Get a report of stats & errors accumulated from all layers */
	gerbv_stats_t *stat = generate_gerber_analysis();

	/* General info report */
	GString *general_report_str = g_string_new(NULL);
	if (stat->layer_count == 0) {
		g_string_printf(general_report_str,
				_("No Gerber layers visible!"));
	} else {
		if (stat->error_list->error_text == NULL) {
			g_string_printf(general_report_str,
				ngettext("No errors found in %d visible "
					"Gerber layer.",
					"No errors found in %d visible "
					"Gerber layers.",
					stat->layer_count),
				stat->layer_count);
		} else {
			g_string_printf(general_report_str,
				ngettext("Found errors in %d visible "
					"Gerber layer.",
					"Found errors in %d visible "
					"Gerber layers.",
					stat->layer_count),
				stat->layer_count);
		}
	}

	GtkWidget *general_label = gtk_label_new(general_report_str->str);
	g_string_free(general_report_str, TRUE);
	gtk_misc_set_alignment(GTK_MISC(general_label), 0, 0);
	gtk_misc_set_padding(GTK_MISC(general_label), 7, 7);
	gtk_label_set_selectable(GTK_LABEL(general_label), TRUE);

	struct table *general_table;
	
	general_table = table_new_with_columns(3,
			_("Layer"), G_TYPE_UINT, _("Type"), G_TYPE_STRING,
			_("Description"), G_TYPE_STRING);
	table_set_column_align(general_table, 0, 1.0);

	for (i = 0; i <= mainProject->last_loaded; i++) {
		gerbv_fileinfo_t **files = mainProject->file;

		if (!files[i]
		||  !files[i]->isVisible
		||   files[i]->image->layertype != GERBV_LAYERTYPE_RS274X)
			continue;

		table_add_row(general_table, i + 1,
				_("RS-274X file"), files[i]->fullPathname);

		str = g_strdup_printf(_("%g x %g %s"),
				screen_units(fabs(files[i]->image->info->max_x -
						files[i]->image->info->min_x)),
				screen_units(fabs(files[i]->image->info->max_y -
						files[i]->image->info->min_y)),
				screen_units_str());
		table_add_row(general_table, i + 1, _("Bounding size"), str);
		g_free(str);

		/* Check error report on layer */
		if (stat->layer_count > 0
		&&  stat->error_list->error_text != NULL) {
			for (gerbv_error_list_t *err_list = stat->error_list;
					err_list != NULL;
					err_list = err_list->next) {

				if (i != err_list->layer - 1)
					continue;

				table_add_row(general_table, err_list->layer,
					error_type_string(err_list->type),
					err_list->error_text);
			}
		}
	}

	/* G codes on active layers */
	GtkWidget *G_report_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(G_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *G_table =
		table_new_with_columns(3, _("Code"), G_TYPE_STRING,
			pgettext("table", "Count"), G_TYPE_UINT,
			_("Note"), G_TYPE_STRING);
	table_set_column_align(G_table, 0, 1.0);
	table_set_column_align(G_table, 1, 1.0);
	gtk_tree_view_set_headers_clickable(
			GTK_TREE_VIEW(G_table->widget), TRUE);

	table_add_row(G_table, "G00", stat->G0,  _(gerber_g_code_name(0)));
	table_add_row(G_table, "G01", stat->G1,  _(gerber_g_code_name(1)));
	table_add_row(G_table, "G02", stat->G2,  _(gerber_g_code_name(2)));
	table_add_row(G_table, "G03", stat->G3,  _(gerber_g_code_name(3)));
	table_add_row(G_table, "G04", stat->G4,  _(gerber_g_code_name(4)));
	table_add_row(G_table, "G10", stat->G10, _(gerber_g_code_name(10)));
	table_add_row(G_table, "G11", stat->G11, _(gerber_g_code_name(11)));
	table_add_row(G_table, "G12", stat->G12, _(gerber_g_code_name(12)));
	table_add_row(G_table, "G36", stat->G36, _(gerber_g_code_name(36)));
	table_add_row(G_table, "G37", stat->G37, _(gerber_g_code_name(37)));
	table_add_row(G_table, "G54", stat->G54, _(gerber_g_code_name(54)));
	table_add_row(G_table, "G55", stat->G55, _(gerber_g_code_name(55)));
	table_add_row(G_table, "G70", stat->G70, _(gerber_g_code_name(70)));
	table_add_row(G_table, "G71", stat->G71, _(gerber_g_code_name(71)));
	table_add_row(G_table, "G74", stat->G74, _(gerber_g_code_name(74)));
	table_add_row(G_table, "G75", stat->G75, _(gerber_g_code_name(75)));
	table_add_row(G_table, "G90", stat->G90, _(gerber_g_code_name(90)));
	table_add_row(G_table, "G91", stat->G91, _(gerber_g_code_name(91)));
	table_add_row(G_table, "", stat->G_unknown, _("unknown G-codes"));

	table_set_sortable(G_table);
	gtk_container_add(GTK_CONTAINER(G_report_window), G_table->widget);

	/* D codes on active layers */
	GtkWidget *D_report_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(D_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *D_table =
		table_new_with_columns(3, _("Code"), G_TYPE_STRING,
			pgettext("table", "Count"), G_TYPE_UINT,
			_("Note"), G_TYPE_STRING);
	table_set_column_align(D_table, 0, 1.0);
	table_set_column_align(D_table, 1, 1.0);
	gtk_tree_view_set_headers_clickable(
			GTK_TREE_VIEW(D_table->widget), TRUE);
	table_add_row(D_table, "D01", stat->D1, _(gerber_d_code_name(1)));
	table_add_row(D_table, "D02", stat->D2, _(gerber_d_code_name(2)));
	table_add_row(D_table, "D03", stat->D3, _(gerber_d_code_name(3)));
	table_add_row(D_table, "", stat->D_unknown, _("unknown D-codes"));
	table_add_row(D_table, "", stat->D_error, _("D-code errors"));

	table_set_sortable(D_table);
	gtk_container_add(GTK_CONTAINER(D_report_window), D_table->widget);

	/* M codes on active layers */
	GtkWidget *M_report_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(M_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *M_table =
		table_new_with_columns(3, _("Code"), G_TYPE_STRING,
			pgettext("table", "Count"), G_TYPE_UINT,
			_("Note"), G_TYPE_STRING);
	table_set_column_align(M_table, 0, 1.0);
	table_set_column_align(M_table, 1, 1.0);
	gtk_tree_view_set_headers_clickable(
			GTK_TREE_VIEW(M_table->widget), TRUE);
	table_add_row(M_table, "M00", stat->M0, _(gerber_m_code_name(0)));
	table_add_row(M_table, "M01", stat->M1, _(gerber_m_code_name(1)));
	table_add_row(M_table, "M02", stat->M2, _(gerber_m_code_name(2)));
	table_add_row(M_table, "", stat->M_unknown, _("unknown M-codes"));

	table_set_sortable(M_table);
	gtk_container_add(GTK_CONTAINER(M_report_window), M_table->widget);

	/* Misc codes */
	GtkWidget *misc_report_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(misc_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *misc_table =
		table_new_with_columns(2, _("Code"), G_TYPE_STRING,
				pgettext("table", "Count"), G_TYPE_UINT);
	table_set_column_align(misc_table, 1, 1.0);
	gtk_tree_view_set_headers_clickable(
			GTK_TREE_VIEW(misc_table->widget), TRUE);
	table_add_row(misc_table, "X", stat->X);
	table_add_row(misc_table, "Y", stat->Y);
	table_add_row(misc_table, "I", stat->I);
	table_add_row(misc_table, "J", stat->J);
	table_add_row(misc_table, "*", stat->star);
	table_add_row(misc_table, _("Unknown"), stat->unknown);

	table_set_sortable(misc_table);
	gtk_container_add(GTK_CONTAINER(misc_report_window),
			misc_table->widget);

	/* Apertures definition in input files */
	GtkWidget *aperture_def_report_window =
		gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(aperture_def_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	if (stat->aperture_list->number == -1) {
		GtkWidget *aperture_def_label = gtk_label_new(
			_("No aperture definitions found in active Gerber file(s)!"));
		gtk_misc_set_alignment(GTK_MISC(aperture_def_label), 0, 0);
		gtk_misc_set_padding(GTK_MISC(aperture_def_label), 7, 7);
		gtk_label_set_selectable(GTK_LABEL(aperture_def_label), TRUE);
		gtk_scrolled_window_add_with_viewport(
				GTK_SCROLLED_WINDOW(aperture_def_report_window),
				aperture_def_label);
	} else {
		struct table *aperture_def_table = table_new_with_columns(6,
				_("Layer"), G_TYPE_UINT,
				_("D code"), G_TYPE_STRING,
				_("Aperture"), G_TYPE_STRING,
				_("Param[0]"), G_TYPE_DOUBLE,
				_("Param[1]"), G_TYPE_DOUBLE,
				_("Param[2]"), G_TYPE_DOUBLE);
		table_set_column_align(aperture_def_table, 0, 1.0);
		table_set_column_align(aperture_def_table, 1, 1.0);
		gtk_tree_view_set_headers_clickable(
				GTK_TREE_VIEW(aperture_def_table->widget), TRUE);
		gtk_tree_view_set_search_column(
				GTK_TREE_VIEW(aperture_def_table->widget), 1);

		GString *gstr = g_string_new(NULL);
		for (aperture_list = stat->aperture_list;
				aperture_list != NULL;
				aperture_list = aperture_list->next) {
			g_string_printf(gstr, "D%d", aperture_list->number);
			table_add_row(aperture_def_table,
				aperture_list->layer,
				gstr->str,
				_(gerbv_aperture_type_name(
					aperture_list->type)),
				aperture_list->parameter[0],
				aperture_list->parameter[1],
				aperture_list->parameter[2]);
		}
		g_string_free(gstr, TRUE);
		table_set_sortable(aperture_def_table);
		gtk_container_add(GTK_CONTAINER(aperture_def_report_window),
				aperture_def_table->widget);
	}

	/* Gerber aperture usage on active layers */
	GtkWidget *aperture_usage_report_window =
		gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(aperture_usage_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	unsigned int aperture_count = 0;

	if (stat->D_code_list->number == -1) {
		GtkWidget *aperture_usage_label = gtk_label_new(
			_("No apertures used in Gerber file(s)!"));
		gtk_misc_set_alignment(GTK_MISC(aperture_usage_label), 0, 0);
		gtk_misc_set_padding(GTK_MISC(aperture_usage_label), 7, 7);
		gtk_label_set_selectable(GTK_LABEL(aperture_usage_label), TRUE);
		gtk_scrolled_window_add_with_viewport(
			GTK_SCROLLED_WINDOW(aperture_usage_report_window),
			aperture_usage_label);
	} else {
		struct table *aperture_usage_table = table_new_with_columns(2,
				_("Code"), G_TYPE_STRING,
				pgettext("table", "Count"), G_TYPE_UINT);
		table_set_column_align(aperture_usage_table, 0, 1.0);
		table_set_column_align(aperture_usage_table, 1, 1.0);
		gtk_tree_view_set_headers_clickable(
			GTK_TREE_VIEW(aperture_usage_table->widget), TRUE);

		GString *gstr = g_string_new(NULL);
		for (aperture_list = stat->D_code_list;
				aperture_list != NULL;
				aperture_list = aperture_list->next) {
			g_string_printf(gstr, "D%d", aperture_list->number);
			table_add_row(aperture_usage_table,
					gstr->str,
					aperture_list->count);
			aperture_count += aperture_list->count;
		}
		g_string_free(gstr, TRUE);
		table_add_row(aperture_usage_table, _("Total"), aperture_count);
		table_set_sortable(aperture_usage_table);
		gtk_container_add( GTK_CONTAINER(aperture_usage_report_window),
				aperture_usage_table->widget);
	}

	/* Create top level dialog window for report */
	GtkWidget *analyze_active_gerbers;
	analyze_active_gerbers = gtk_dialog_new_with_buttons(
			_("Gerber codes report on visible layers"),
			NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_container_set_border_width(GTK_CONTAINER (analyze_active_gerbers), 5);

	gtk_dialog_set_default_response (GTK_DIALOG(analyze_active_gerbers),
			GTK_RESPONSE_ACCEPT);
	g_signal_connect_after (G_OBJECT(analyze_active_gerbers),
			"response",
			G_CALLBACK (gtk_widget_destroy),
			GTK_WIDGET(analyze_active_gerbers));

	/* Put general report text into scrolled window */
	GtkWidget *general_report_window =
		gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(general_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *vbox = gtk_vbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), general_label, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), general_table->widget, 0, 0, 0);
	gtk_scrolled_window_add_with_viewport(
			GTK_SCROLLED_WINDOW(general_report_window), vbox);

	/* Create tabbed notebook widget and add report widgets. */
	GtkWidget *notebook = gtk_notebook_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(general_report_window),
			gtk_label_new(_("General")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(G_report_window),
			gtk_label_new(_("G codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(D_report_window),
			gtk_label_new(_("D codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(M_report_window),
			gtk_label_new(_("M codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(misc_report_window),
			gtk_label_new(_("Misc. codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(aperture_def_report_window),
			gtk_label_new(_("Aperture definitions")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(aperture_usage_report_window),
			gtk_label_new(_("Aperture usage")));

	/* Now put notebook into dialog window and show the whole thing */
	gtk_container_add(
			GTK_CONTAINER(GTK_DIALOG(analyze_active_gerbers)->vbox),
			GTK_WIDGET(notebook));

	if (screen.settings) {
		analyze_window_size_restore(analyze_active_gerbers);
		g_signal_connect (G_OBJECT(analyze_active_gerbers), "response",
				G_CALLBACK (analyze_window_size_store),
				GTK_WIDGET(analyze_active_gerbers));
	} else {
		gtk_window_set_default_size(GTK_WINDOW(analyze_active_gerbers),
						640, 320);
	}

	gtk_widget_show_all(analyze_active_gerbers);

	gerbv_stats_destroy(stat);	
}