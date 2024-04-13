callbacks_analyze_active_drill_activate(GtkMenuItem *menuitem,
					gpointer user_data)
{
	gchar *str;
	int i;

	gerbv_drill_stats_t *stat = generate_drill_analysis();

	/* General info report */
	GString *general_report_str = g_string_new(NULL);
	if (stat->layer_count == 0) {
		g_string_printf(general_report_str,
				_("No drill layers visible!"));
	} else {
		if (stat->error_list->error_text == NULL) {
			g_string_printf(general_report_str,
				ngettext("No errors found in %d visible "
					"drill layer.",
					"No errors found in %d visible "
					"drill layers.",
					stat->layer_count),
				stat->layer_count);
		} else {
			g_string_printf(general_report_str,
				ngettext("Found errors found in %d visible "
					"drill layer.",
					"Found errors found in %d visible "
					"drill layers.",
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
		||   files[i]->image->layertype != GERBV_LAYERTYPE_DRILL)
			continue;

		table_add_row(general_table, i + 1,
				_("Excellon file"), files[i]->fullPathname);

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

	table_add_row(G_table, "G00", stat->G00, _(drill_g_code_name(0)));
	table_add_row(G_table, "G01", stat->G01, _(drill_g_code_name(1)));
	table_add_row(G_table, "G02", stat->G02, _(drill_g_code_name(2)));
	table_add_row(G_table, "G03", stat->G03, _(drill_g_code_name(3)));
	table_add_row(G_table, "G04", stat->G04, _(drill_g_code_name(4)));
	table_add_row(G_table, "G05", stat->G05, _(drill_g_code_name(5)));
	table_add_row(G_table, "G85", stat->G85, _(drill_g_code_name(85)));
	table_add_row(G_table, "G90", stat->G90, _(drill_g_code_name(90)));
	table_add_row(G_table, "G91", stat->G91, _(drill_g_code_name(91)));
	table_add_row(G_table, "G93", stat->G93, _(drill_g_code_name(93)));
	table_add_row(G_table, "", stat->G_unknown, _("unknown G-codes"));

	table_set_sortable(G_table);
	gtk_container_add(GTK_CONTAINER(G_report_window), G_table->widget);

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
	table_add_row(M_table, "M00", stat->M00, _(drill_m_code_name(0)));
	table_add_row(M_table, "M01", stat->M01, _(drill_m_code_name(1)));
	table_add_row(M_table, "M18", stat->M18, _(drill_m_code_name(18)));
	table_add_row(M_table, "M25", stat->M25, _(drill_m_code_name(25)));
	table_add_row(M_table, "M30", stat->M30, _(drill_m_code_name(30)));
	table_add_row(M_table, "M45", stat->M45, _(drill_m_code_name(45)));
	table_add_row(M_table, "M47", stat->M47, _(drill_m_code_name(47)));
	table_add_row(M_table, "M48", stat->M48, _(drill_m_code_name(48)));
	table_add_row(M_table, "M71", stat->M71, _(drill_m_code_name(71)));
	table_add_row(M_table, "M72", stat->M72, _(drill_m_code_name(72)));
	table_add_row(M_table, "M95", stat->M95, _(drill_m_code_name(95)));
	table_add_row(M_table, "M97", stat->M97, _(drill_m_code_name(97)));
	table_add_row(M_table, "M98", stat->M98, _(drill_m_code_name(98)));
	table_add_row(M_table, "", stat->M_unknown, _("unknown M-codes"));

	table_set_sortable(M_table);
	gtk_container_add(GTK_CONTAINER(M_report_window), M_table->widget);

	/* Misc codes */
	GtkWidget *misc_report_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(misc_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *misc_table =
		table_new_with_columns(2,
				/* Count is string for value hide. */
				pgettext("table", "Count"), G_TYPE_STRING,
				_("Code"), G_TYPE_STRING);
	table_set_column_align(misc_table, 0, 1.0);
	str = g_strdup_printf("%d", stat->comment);
	table_add_row(misc_table, str,_("Comments"));
	g_free(str);
	str = g_strdup_printf("%d", stat->unknown);
	table_add_row(misc_table, str, _("Unknown codes"));
	g_free(str);
	str = g_strdup_printf("%d", stat->R);
	table_add_row(misc_table, str, _("Repeat hole (R)"));
	g_free(str);
	if (stat->detect != NULL ) {
		table_add_row(misc_table, "", stat->detect);
	}

	table_set_sortable(misc_table);
	gtk_container_add(GTK_CONTAINER(misc_report_window),
			misc_table->widget);

	/* Drill usage on active layers */
	GtkWidget *drill_usage_report_window =
			gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(drill_usage_report_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	struct table *drill_usage_table = table_new_with_columns(4,
			_("Drill no."), G_TYPE_UINT,
			_("Dia."), G_TYPE_DOUBLE,
			_("Units"), G_TYPE_STRING,
			pgettext("table", "Count"), G_TYPE_UINT);

	table_set_column_align(drill_usage_table, 0, 1.0);
	table_set_column_align(drill_usage_table, 3, 1.0);
	gtk_tree_view_set_headers_clickable(
		GTK_TREE_VIEW(drill_usage_table->widget), TRUE);

	gerbv_drill_list_t *drill_list;
	for (drill_list = stat->drill_list;
			drill_list != NULL; drill_list = drill_list->next) {
		if (drill_list->drill_num == -1)
			break;	/* No drill list */

		table_add_row(drill_usage_table,
				drill_list->drill_num,
				drill_list->drill_size,
				drill_list->drill_unit,
				drill_list->drill_count);
	}

	table_set_sortable(drill_usage_table);
	gtk_container_add(GTK_CONTAINER(drill_usage_report_window),
			drill_usage_table->widget);

	/* Create top level dialog window for report */
	GtkWidget *analyze_active_drill;
	analyze_active_drill = gtk_dialog_new_with_buttons(
			_("Drill codes report on visible layers"),
			NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (analyze_active_drill), 5);

	gtk_dialog_set_default_response (GTK_DIALOG(analyze_active_drill),
			GTK_RESPONSE_ACCEPT);
	g_signal_connect_after (G_OBJECT(analyze_active_drill),
			"response",
			G_CALLBACK (gtk_widget_destroy),
			GTK_WIDGET(analyze_active_drill));

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
			GTK_WIDGET(M_report_window),
			gtk_label_new(_("M codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(misc_report_window),
			gtk_label_new(_("Misc. codes")));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(drill_usage_report_window),
			gtk_label_new(_("Drill usage")));

	/* Now put notebook into dialog window and show the whole thing */
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(analyze_active_drill)->vbox),
			GTK_WIDGET(notebook));

	if (screen.settings) {
		analyze_window_size_restore(analyze_active_drill);
		g_signal_connect (G_OBJECT(analyze_active_drill), "response",
				G_CALLBACK (analyze_window_size_store),
				GTK_WIDGET(analyze_active_drill));
	} else {
		gtk_window_set_default_size(GTK_WINDOW(analyze_active_drill),
						640, 320);
	}

	gtk_widget_show_all(analyze_active_drill);

	gerbv_drill_stats_destroy(stat);
}