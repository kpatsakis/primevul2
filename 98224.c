IW_IMPL(void) iw_translate(struct iw_context *ctx, unsigned int flags,
	char *dst, size_t dstlen, const char *src)
{
	int ret;

	dst[0]='\0';

	if(ctx && ctx->translate_fn) {
		ret = (*ctx->translate_fn)(ctx,flags,dst,dstlen,src);
	}
	else {
		ret = 0;
	}

	if(!ret) {
		iw_strlcpy(dst,src,dstlen);
	}
}
