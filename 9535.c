visible_file_name(gchar **file_name, gchar **dir_name,
		gerbv_layertype_t layer_type, /* -1 for all types */
		const gchar *file_extension,
		const gchar *untitled_file_extension)
{
	unsigned int count = 0;
	gerbv_fileinfo_t *first_vis_file = NULL;

	for (int i = 0; i < mainProject->max_files; ++i) {
		if (mainProject->file[i]
		&&  mainProject->file[i]->isVisible
		&&  (layer_type == (gerbv_layertype_t)-1
		  || layer_type == mainProject->file[i]->image->layertype)) {
			if (first_vis_file == NULL) {
				first_vis_file = mainProject->file[i];
				/* Always directory of first visible file */
				if (dir_name)
					*dir_name = g_path_get_dirname (
						first_vis_file->fullPathname);
			}

			if (++count == 2 && file_name) {
				*file_name = g_strdup_printf("%s%s",
						pgettext("file name",
							"untitled"),
						untitled_file_extension);
			}
		}
	}

	if (count == 1 && file_name)
		*file_name = g_strdup_printf("%s%s",
			first_vis_file->name, file_extension);

	return count;
}