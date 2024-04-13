aperture_report(gerbv_aperture_t *apertures[], int aperture_num,
		double x, double y, gerbv_image_t *img, gerbv_project_t *prj)
{
	gerbv_aperture_type_t type = apertures[aperture_num]->type;
	double *params = apertures[aperture_num]->parameter;
	gerbv_simplified_amacro_t *sam = apertures[aperture_num]->simplified;

	g_message (_("    Aperture used: D%d"), aperture_num);
	g_message (_("    Aperture type: %s"),
		(type == GERBV_APTYPE_MACRO)?
			_(gerbv_aperture_type_name(sam->type)):
			_(gerbv_aperture_type_name(type)));

	switch (type) {
	case GERBV_APTYPE_CIRCLE:
		g_message (_("    Diameter: %g %s"),
			screen_units(params[0]),
			screen_units_str());
		break;
	case GERBV_APTYPE_RECTANGLE:
	case GERBV_APTYPE_OVAL:
		g_message (_("    Dimensions: %gx%g %s"),
			screen_units(params[0]),
			screen_units(params[1]),
			screen_units_str());
		break;
	case GERBV_APTYPE_MACRO: {
		switch (sam->type) {
		case GERBV_APTYPE_MACRO_CIRCLE:
			g_message (_("    Diameter: %g %s"),
				screen_units(sam->parameter[CIRCLE_DIAMETER]),
				screen_units_str());
			x += sam->parameter[CIRCLE_CENTER_X];
			y += sam->parameter[CIRCLE_CENTER_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Center: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			break;

		case GERBV_APTYPE_MACRO_OUTLINE:
			g_message (_("    Number of points: %g"),
				sam->parameter[OUTLINE_NUMBER_OF_POINTS]);
			x += sam->parameter[OUTLINE_FIRST_X];
			y += sam->parameter[OUTLINE_FIRST_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Start: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
				sam->parameter[OUTLINE_ROTATION_IDX(sam->parameter)]);
			break;

		case GERBV_APTYPE_MACRO_POLYGON:
			g_message (_("    Number of points: %g"),
				sam->parameter[POLYGON_NUMBER_OF_POINTS]);
			g_message (_("    Diameter: %g %s"),
				screen_units(sam->parameter[POLYGON_DIAMETER]),
				screen_units_str());
			x += sam->parameter[POLYGON_CENTER_X];
			y += sam->parameter[POLYGON_CENTER_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Center: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
				sam->parameter[POLYGON_ROTATION]);
			break;

		case GERBV_APTYPE_MACRO_MOIRE:
			g_message (_("    Outside diameter: %g %s"),
				screen_units(sam->parameter[MOIRE_OUTSIDE_DIAMETER]),
				screen_units_str());
			g_message (_("    Ring thickness: %g %s"),
				screen_units(sam->parameter[MOIRE_CIRCLE_THICKNESS]),
				screen_units_str());
			g_message (_("    Gap width: %g %s"),
				screen_units(sam->parameter[MOIRE_GAP_WIDTH]),
				screen_units_str());
			g_message (_("    Number of rings: %g"),
				sam->parameter[MOIRE_NUMBER_OF_CIRCLES]);
			g_message (_("    Crosshair thickness: %g %s"),
				screen_units(
					sam->parameter[MOIRE_CROSSHAIR_THICKNESS]),
				screen_units_str());
			g_message (_("    Crosshair length: %g %s"),
				screen_units(sam->parameter[MOIRE_CROSSHAIR_LENGTH]),
				screen_units_str());
			x += sam->parameter[MOIRE_CENTER_X];
			y += sam->parameter[MOIRE_CENTER_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Center: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
				sam->parameter[MOIRE_ROTATION]);
			break;

		case GERBV_APTYPE_MACRO_THERMAL:
			g_message (_("    Outside diameter: %g %s"),
				screen_units(sam->parameter[THERMAL_OUTSIDE_DIAMETER]),
				screen_units_str());
			g_message (_("    Inside diameter: %g %s"),
				screen_units(sam->parameter[THERMAL_INSIDE_DIAMETER]),
				screen_units_str());
			g_message (_("    Crosshair thickness: %g %s"),
				screen_units(
					sam->parameter[THERMAL_CROSSHAIR_THICKNESS]),
				screen_units_str());
			x += sam->parameter[THERMAL_CENTER_X];
			y += sam->parameter[THERMAL_CENTER_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Center: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
				sam->parameter[THERMAL_ROTATION]);
			break;

		case GERBV_APTYPE_MACRO_LINE20:
			g_message (_("    Width: %g %s"),
				screen_units(sam->parameter[LINE20_WIDTH]),
				screen_units_str());
			x += sam->parameter[LINE20_START_X];
			y += sam->parameter[LINE20_START_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Start: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			x += sam->parameter[LINE20_END_X];
			y += sam->parameter[LINE20_END_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Stop: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
					sam->parameter[LINE20_ROTATION]);
			break;

		case GERBV_APTYPE_MACRO_LINE21:
			g_message (_("    Width: %g %s"),
				screen_units(sam->parameter[LINE21_WIDTH]),
				screen_units_str());
			g_message (_("    Height: %g %s"),
				screen_units(sam->parameter[LINE21_HEIGHT]),
				screen_units_str());
			x += sam->parameter[LINE21_CENTER_X];
			y += sam->parameter[LINE21_CENTER_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Center: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
					sam->parameter[LINE21_ROTATION]);
			break;

		case GERBV_APTYPE_MACRO_LINE22:
			g_message (_("    Width: %g %s"),
				screen_units(sam->parameter[LINE22_WIDTH]),
				screen_units_str());
			g_message (_("    Height: %g %s"),
				screen_units(sam->parameter[LINE22_HEIGHT]),
				screen_units_str());
			x += sam->parameter[LINE22_LOWER_LEFT_X];
			y += sam->parameter[LINE22_LOWER_LEFT_Y];
			gerbv_transform_coord_for_image(&x, &y, img, prj);
			g_message (_("    Lower left: (%g, %g) %s"),
				screen_units(x), screen_units(y),
				screen_units_str());
			g_message (_("    Rotation: %g deg"),
					sam->parameter[LINE22_ROTATION]);
			break;

		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}