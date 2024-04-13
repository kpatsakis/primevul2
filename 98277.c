static int iwgif_read_extension(struct iwgifrcontext *rctx)
{
	int retval=0;
	iw_byte ext_type;

	if(!iwgif_read(rctx,rctx->rbuf,1)) goto done;
	ext_type=rctx->rbuf[0];

	switch(ext_type) {
	case 0xf9:
		if(rctx->page == rctx->pages_seen+1) {
			if(!iwgif_read_graphic_control_ext(rctx)) goto done;
		}
		else {
			if(!iwgif_skip_subblocks(rctx)) goto done;
		}
		break;
	default:
		if(!iwgif_skip_subblocks(rctx)) goto done;
	}

	retval=1;
done:
	return retval;
}
