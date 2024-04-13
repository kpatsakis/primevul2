callbacks_update_statusbar_measured_distance (gdouble dx, gdouble dy){
	gdouble delta = hypot(dx, dy);
	
	if (screen.unit == GERBV_MILS) {
	    utf8_snprintf(screen.statusbar.diststr, MAX_DISTLEN,
		     _("Measured distance: %8.2f mils (%8.2f x, %8.2f y)"),
		     COORD2MILS(delta), COORD2MILS(dx), COORD2MILS(dy));
	} 
	else if (screen.unit == GERBV_MMS) {
	    utf8_snprintf(screen.statusbar.diststr, MAX_DISTLEN,
		     _("Measured distance: %8.3f mm (%8.3f x, %8.3f y)"),
		     COORD2MMS(delta), COORD2MMS(dx), COORD2MMS(dy));
	}
	else {
	    utf8_snprintf(screen.statusbar.diststr, MAX_DISTLEN,
		     _("Measured distance: %4.5f inches (%4.5f x, %4.5f y)"),
		     COORD2INS(delta), COORD2INS(dx), COORD2INS(dy));
	}
	callbacks_update_statusbar();
}