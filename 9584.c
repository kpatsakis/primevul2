callbacks_handle_log_messages(const gchar *log_domain, GLogLevelFlags log_level,
		      const gchar *message, gpointer user_data)
{
	GtkTextBuffer *textbuffer = NULL;
	GtkTextIter iter;
	GtkTextTag *tag;
	GtkTextMark *StartMark = NULL, *StopMark = NULL;
	GtkTextIter StartIter, StopIter;
	GtkWidget *dialog, *label;

	if (!screen.win.messageTextView)
		return;
		
	textbuffer = gtk_text_view_get_buffer((GtkTextView*)screen.win.messageTextView);

	/* create a mark for the end of the text. */
	gtk_text_buffer_get_end_iter(textbuffer, &iter);

	/* get the current end position of the text (it will be the
	      start of the new text. */
	StartMark = gtk_text_buffer_create_mark(textbuffer,
					    "NewTextStart", &iter, TRUE);

	tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                              "blue_foreground");
	/* the tag does not exist: create it and let them exist in the tag table.*/
	if (tag == NULL)    {
		tag = gtk_text_buffer_create_tag(textbuffer, "black_foreground",
		                              "foreground", "black", NULL);
		tag = gtk_text_buffer_create_tag(textbuffer, "blue_foreground",
		                              "foreground", "blue", NULL);
		tag = gtk_text_buffer_create_tag(textbuffer, "red_foreground",
		                              "foreground", "red", NULL);
		tag = gtk_text_buffer_create_tag(textbuffer, "darkred_foreground",
		                              "foreground", "darkred", NULL);
		tag = gtk_text_buffer_create_tag(textbuffer, "darkblue_foreground",
		                              "foreground", "darkblue", NULL);
		tag = gtk_text_buffer_create_tag (textbuffer, "darkgreen_foreground",
		                              "foreground", "darkgreen", NULL);
		tag = gtk_text_buffer_create_tag (textbuffer,
		                              "saddlebrown_foreground",
		                              "foreground", "saddlebrown", NULL);
	}

	/* 
	* See rgb.txt for the color names definition 
	* (on my PC it is on /usr/X11R6/lib/X11/rgb.txt)
	*/
	switch (log_level & G_LOG_LEVEL_MASK) {
	case G_LOG_LEVEL_ERROR:
	/* a message of this kind aborts the application calling abort() */
	      tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                                    "red_foreground");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(screen.win.sidepane_notebook), 1);
	      gtk_widget_show(screen.win.sidepane_notebook);
		break;
	case G_LOG_LEVEL_CRITICAL:
	      tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                                    "red_foreground");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(screen.win.sidepane_notebook), 1);
	      gtk_widget_show(screen.win.sidepane_notebook);
		break;
	case G_LOG_LEVEL_WARNING:
	      tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                                    "darkred_foreground");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(screen.win.sidepane_notebook), 1);
	      gtk_widget_show(screen.win.sidepane_notebook);
		break;
	case G_LOG_LEVEL_MESSAGE:
	      tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                                    "darkblue_foreground");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(screen.win.sidepane_notebook), 1);
	      gtk_widget_show(screen.win.sidepane_notebook);
		break;
	case G_LOG_LEVEL_INFO:
	      tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
	                                    "darkgreen_foreground");
		break;
	case G_LOG_LEVEL_DEBUG:
		tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textbuffer),
					"saddlebrown_foreground");
		break;
	default:
	      tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table(textbuffer),
	                                    "black_foreground");
		break;
	}

	/*
	* Fatal aborts application. We will try to get the message out anyhow.
	*/
	if (log_level & G_LOG_FLAG_FATAL) {
		fprintf(stderr, _("Fatal error: %s\n"), message);

		/* Try to show dialog box with error message */
		dialog = gtk_dialog_new_with_buttons(_("Fatal Error"),
			NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);

		label = gtk_label_new(g_strdup_printf(_("Fatal error: %s"), message));
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
				label);
		gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
				gtk_label_new(_("\nGerbv will be closed now!")));

		gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);

		gtk_widget_show_all(dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));
	}

	gtk_text_buffer_insert(textbuffer, &iter, message, -1);
	gtk_text_buffer_insert(textbuffer, &iter, "\n", -1);

	/* Scroll view to inserted text */
	g_signal_emit_by_name(textbuffer, "paste-done", NULL);

	gtk_text_buffer_get_end_iter(textbuffer, &iter);

	StopMark = gtk_text_buffer_create_mark(textbuffer,
					   "NewTextStop", &iter, TRUE);

	gtk_text_buffer_get_iter_at_mark(textbuffer, &StartIter, StartMark);
	gtk_text_buffer_get_iter_at_mark(textbuffer, &StopIter, StopMark);

	gtk_text_buffer_apply_tag(textbuffer, tag, &StartIter, &StopIter);
}