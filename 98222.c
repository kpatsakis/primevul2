IW_IMPL(void) iw_set_value_dbl(struct iw_context *ctx, int code, double n)
{
	switch(code) {
	case IW_VAL_WEBP_QUALITY:
		iw_set_option(ctx, "webp:quality", iwpvt_strdup_dbl(ctx, n)); 
		break;
	case IW_VAL_TRANSLATE_X:
		ctx->resize_settings[IW_DIMENSION_H].translate = n;
		break;
	case IW_VAL_TRANSLATE_Y:
		ctx->resize_settings[IW_DIMENSION_V].translate = n;
		break;
	}
}
