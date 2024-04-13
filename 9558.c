callbacks_align_files_from_sel_clicked (
		GtkMenuItem *menu_item, gpointer user_data)
{
	gerbv_fileinfo_t *fi[2];
	gerbv_selection_item_t item[2];
	gerbv_net_t *net;
	gerbv_selection_info_t *sel_info = &screen.selectionInfo;
	int align_second_to_first = GPOINTER_TO_INT(user_data);
	gdouble x[2], y[2];
	int i;

	if (selection_length (sel_info) != 2)
		return;

	item[0] = selection_get_item_by_index(sel_info, 0);
	item[1] = selection_get_item_by_index(sel_info, 1);

	fi[0] = gerbv_get_fileinfo_for_image (item[0].image, mainProject);
	fi[1] = gerbv_get_fileinfo_for_image (item[1].image, mainProject);

	if (fi[0] == NULL || fi[1] == NULL || fi[0] == fi[1])
		return;

	/* Calculate aligning coords */
	for (i = 0; i < 2; i++) {
		net = item[i].net;

		switch (net->aperture_state) {
		case GERBV_APERTURE_STATE_FLASH:
			x[i] = net->stop_x;
			y[i] = net->stop_y;
			break;
		case GERBV_APERTURE_STATE_ON:
			switch (net->interpolation) {
			case GERBV_INTERPOLATION_LINEARx1:
			case GERBV_INTERPOLATION_LINEARx10:
			case GERBV_INTERPOLATION_LINEARx01:
			case GERBV_INTERPOLATION_LINEARx001:
				x[i] = (net->stop_x + net->start_x)/2;
				y[i] = (net->stop_y + net->start_y)/2;
				break;
			case GERBV_INTERPOLATION_CW_CIRCULAR:
			case GERBV_INTERPOLATION_CCW_CIRCULAR:
				x[i] = net->cirseg->cp_x;
				y[i] = net->cirseg->cp_y;
				break;
			default:
				GERB_COMPILE_ERROR (_("Can't align by this "
							"type of object"));
				return;
			}
			break;
		default:
			GERB_COMPILE_ERROR (_("Can't align by this "
						"type of object"));
			return;
		}

		gerbv_transform_coord_for_image(x + i, y + i,
				item[i].image, mainProject);
	}

	if (align_second_to_first) {
		fi[1]->transform.translateX += x[0] - x[1];
		fi[1]->transform.translateY += y[0] - y[1];
	} else {
		fi[0]->transform.translateX += x[1] - x[0];
		fi[0]->transform.translateY += y[1] - y[0];
	}

	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree ();
}