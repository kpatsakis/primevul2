callbacks_update_ruler_scales (void) {
	double xStart, xEnd, yStart, yEnd;

	xStart = screenRenderInfo.lowerLeftX;
	yStart = screenRenderInfo.lowerLeftY;
	xEnd = screenRenderInfo.lowerLeftX + (screenRenderInfo.displayWidth / screenRenderInfo.scaleFactorX);
	yEnd = screenRenderInfo.lowerLeftY + (screenRenderInfo.displayHeight / screenRenderInfo.scaleFactorY);
	/* mils can get super crowded with large boards, but inches are too
	   large for most boards.  So, we leave mils in for now and just switch
	   to inches if the scale factor gets too small */
	if (!((screen.unit == GERBV_MILS) && ((screenRenderInfo.scaleFactorX < 80)||(screenRenderInfo.scaleFactorY < 80)))) {
		xStart = callbacks_calculate_actual_distance (xStart);
		xEnd = callbacks_calculate_actual_distance (xEnd);
		yStart = callbacks_calculate_actual_distance (yStart);
		yEnd = callbacks_calculate_actual_distance (yEnd);
	}
	/* make sure the widgets actually exist before setting (in case this gets
	   called before everything is realized */
	if (screen.win.hRuler)
		gtk_ruler_set_range (GTK_RULER (screen.win.hRuler), xStart, xEnd, 0, xEnd - xStart);
	/* reverse y min and max, since the ruler starts at the top */
	if (screen.win.vRuler)
		gtk_ruler_set_range (GTK_RULER (screen.win.vRuler), yEnd, yStart, 0, yEnd - yStart);
}