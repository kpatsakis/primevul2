IW_IMPL(void) iw_set_value(struct iw_context *ctx, int code, int n)
{
	switch(code) {
	case IW_VAL_API_VERSION:
		ctx->caller_api_version = n;
		break;
	case IW_VAL_CVT_TO_GRAYSCALE:
		ctx->to_grayscale = n;
		break;
	case IW_VAL_DISABLE_GAMMA:
		ctx->no_gamma = n;
		break;
	case IW_VAL_NO_CSLABEL:
		ctx->req.suppress_output_cslabel = n;
		break;
	case IW_VAL_INT_CLAMP:
		ctx->intclamp = n;
		break;
	case IW_VAL_EDGE_POLICY_X:
		ctx->resize_settings[IW_DIMENSION_H].edge_policy = n;
		break;
	case IW_VAL_EDGE_POLICY_Y:
		ctx->resize_settings[IW_DIMENSION_V].edge_policy = n;
		break;
	case IW_VAL_PREF_UNITS:
		ctx->pref_units = n;
		break;
	case IW_VAL_GRAYSCALE_FORMULA:
		ctx->grayscale_formula = n;
		break;
	case IW_VAL_INPUT_NATIVE_GRAYSCALE:
		ctx->img1.native_grayscale = n;
		break;
	case IW_VAL_COMPRESSION:
		ctx->req.compression = n;
		break;
	case IW_VAL_PAGE_TO_READ:
		ctx->req.page_to_read = n;
		break;
	case IW_VAL_INCLUDE_SCREEN:
		ctx->req.include_screen = n;
		break;
	case IW_VAL_JPEG_QUALITY:
		iw_set_option(ctx, "jpeg:quality", iwpvt_strdup_dbl(ctx, (double)n));
		break;
	case IW_VAL_JPEG_SAMP_FACTOR_H:
		iw_set_option(ctx, "jpeg:sampling-x", iwpvt_strdup_dbl(ctx, (double)n));
		break;
	case IW_VAL_JPEG_SAMP_FACTOR_V:
		iw_set_option(ctx, "jpeg:sampling-y", iwpvt_strdup_dbl(ctx, (double)n));
		break;
	case IW_VAL_JPEG_ARITH_CODING:
		iw_set_option(ctx, "jpeg:arith", iwpvt_strdup_dbl(ctx, (double)n));
		break;
	case IW_VAL_DEFLATE_CMPR_LEVEL:
		iw_set_option(ctx, "deflate:cmprlevel", iwpvt_strdup_dbl(ctx, (double)n)); 
		break;
	case IW_VAL_OUTPUT_INTERLACED:
		ctx->req.interlaced = n;
		break;
	case IW_VAL_USE_BKGD_LABEL:
		ctx->req.use_bkgd_label_from_file = n;
		break;
	case IW_VAL_BMP_NO_FILEHEADER:
		ctx->req.bmp_no_fileheader = n;
		break;
	case IW_VAL_BMP_VERSION:
		iw_set_option(ctx, "bmp:version", iwpvt_strdup_dbl(ctx, (double)n));
		break;
	case IW_VAL_MAX_WIDTH:
		ctx->max_width = n;
		break;
	case IW_VAL_MAX_HEIGHT:
		ctx->max_height = n;
		break;
	case IW_VAL_NO_BKGD_LABEL:
		ctx->req.suppress_output_bkgd_label = n;
		break;
	case IW_VAL_INTENT:
		ctx->req.output_rendering_intent = n;
		break;
	case IW_VAL_OUTPUT_SAMPLE_TYPE:
		ctx->req.output_sample_type = n;
		break;
	case IW_VAL_OUTPUT_COLOR_TYPE:
		if(n==IW_COLORTYPE_RGB) {
			iw_set_option(ctx, "deflate:colortype", "rgb"); 
		}
		break;
	case IW_VAL_OUTPUT_FORMAT:
		ctx->req.output_format = n;
		break;
	case IW_VAL_NEGATE_TARGET:
		ctx->req.negate_target = n;
		break;
	}
}
