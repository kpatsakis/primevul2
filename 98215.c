IW_IMPL(void) iw_make_srgb_csdescr(struct iw_csdescr *cs, int srgb_intent)
{
	cs->cstype = IW_CSTYPE_SRGB;
	cs->gamma = 0.0;
	cs->srgb_intent = srgb_intent;
}
