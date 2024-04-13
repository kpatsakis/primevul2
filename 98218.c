IW_IMPL(void) iw_set_dither_type(struct iw_context *ctx, int channeltype, int f, int s)
{
	if(channeltype>=0 && channeltype<IW_NUM_CHANNELTYPES) {
		ctx->ditherfamily_by_channeltype[channeltype] = f;
		ctx->dithersubtype_by_channeltype[channeltype] = s;
	}

	switch(channeltype) {
	case IW_CHANNELTYPE_ALL:
		ctx->ditherfamily_by_channeltype[IW_CHANNELTYPE_ALPHA] = f;
		ctx->dithersubtype_by_channeltype[IW_CHANNELTYPE_ALPHA] = s;
	case IW_CHANNELTYPE_NONALPHA:
		ctx->ditherfamily_by_channeltype[IW_CHANNELTYPE_RED] = f;
		ctx->dithersubtype_by_channeltype[IW_CHANNELTYPE_RED] = s;
		ctx->ditherfamily_by_channeltype[IW_CHANNELTYPE_GREEN] = f;
		ctx->dithersubtype_by_channeltype[IW_CHANNELTYPE_GREEN] = s;
		ctx->ditherfamily_by_channeltype[IW_CHANNELTYPE_BLUE] = f;
		ctx->dithersubtype_by_channeltype[IW_CHANNELTYPE_BLUE] = s;
		ctx->ditherfamily_by_channeltype[IW_CHANNELTYPE_GRAY] = f;
		ctx->dithersubtype_by_channeltype[IW_CHANNELTYPE_GRAY] = s;
		break;
	}
}
