callbacks_change_layer_format_clicked  (GtkButton *button, gpointer   user_data)
{
    gerbv_HID_Attribute *attr = NULL;
    int n = 0;
    int i;
    gerbv_HID_Attr_Val * results = NULL;
    gint index = callbacks_get_selected_row_index();
    gchar *type;
    gint rc;
	if (index < 0) {
		show_no_layers_warning ();
		return;
	}
    dprintf ("%s(): index = %d\n", __FUNCTION__, index);
    attr = mainProject->file[index]->image->info->attr_list;
    n =  mainProject->file[index]->image->info->n_attr;
    type =  mainProject->file[index]->image->info->type;
    if (type == NULL) 
	type = N_("Unknown type");

    if (attr == NULL || n == 0) 
	{
	  interface_show_alert_dialog(_("This file type does not currently have any editable features"), 
				      _("Format editing is currently only supported for Excellon drill file formats."),
				      FALSE,
				      NULL);
	  return;
	}

    dprintf ("%s(): n = %d, attr = %p\n", __FUNCTION__, n, attr);
    if (n > 0)
	{
	    if (mainProject->file[index]->layer_dirty) {
		rc = interface_get_alert_dialog_response (
			_("This layer has changed!"),
			_("Editing the file type will reload the layer, "
			"destroying your changes.  Click OK to edit "
			"the file type and destroy your changes, "
			"or Cancel to leave."),
			TRUE, NULL, GTK_STOCK_OK, GTK_STOCK_CANCEL);
		if (rc == 0) return;  /* Return if user hit Cancel */
	    }

	    results = (gerbv_HID_Attr_Val *) malloc (n * sizeof (gerbv_HID_Attr_Val));
	    if (results == NULL)
		GERB_FATAL_ERROR("%s(): malloc failed", __FUNCTION__);
      
	    /* non-zero means cancel was picked */
	    if (attribute_interface_dialog (attr, n, results, 
					    _("Edit file format"), 
					    _(type)))
		{
		    return;
		}
          
    }

    dprintf ("%s(): reloading layer\n", __func__);
    gerbv_revert_file (mainProject, index);

    for (i = 0; i < n; i++)
	{
	    if (results[i].str_value)
		free (results[i].str_value);
	}
    
    if (results)
	free (results);
    render_refresh_rendered_image_on_screen();
    callbacks_update_layer_tree();
}