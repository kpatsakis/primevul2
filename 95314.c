pdf14_cmap_rgb_alpha_direct(frac r, frac g, frac b, frac alpha,	gx_device_color	* pdc,
     const gs_gstate * pgs, gx_device * dev, gs_color_select_t select)
{
    int i, ncomps;
    frac cm_comps[GX_DEVICE_COLOR_MAX_COMPONENTS];
    gx_color_value cv[GX_DEVICE_COLOR_MAX_COMPONENTS];
    gx_color_index color;
    gx_device *trans_device;

     /*  We may be coming from the clist writer which often forwards us the
         target device. If this occurs we actually need to get to the color
         space defined by the transparency group and we use the operators
         defined by the transparency device to do the job. */
    if (pgs->trans_device != NULL){
        trans_device = pgs->trans_device;
    } else {
        trans_device = dev;
    }
    ncomps = trans_device->color_info.num_components;
    /* map to the color model */
    dev_proc(trans_device, get_color_mapping_procs)(trans_device)->map_rgb(trans_device, pgs, r, g, b, cm_comps);
    /* pre-multiply to account for the alpha weighting */
    if (alpha != frac_1) {
#ifdef PREMULTIPLY_TOWARDS_WHITE
        frac alpha_bias = frac_1 - alpha;
#else
        frac alpha_bias = 0;
#endif
        for (i = 0; i < ncomps; i++)
            cm_comps[i] = (frac)((long)cm_comps[i] * alpha) / frac_1 + alpha_bias;
    }

    for (i = 0; i < ncomps; i++)
        cv[i] = frac2cv(gx_map_color_frac(pgs, cm_comps[i], effective_transfer[i]));
    color = dev_proc(trans_device, encode_color)(trans_device, cv);
    /* check if the encoding was successful; we presume failure is rare */
    if (color != gx_no_color_index)
        color_set_pure(pdc, color);
}
