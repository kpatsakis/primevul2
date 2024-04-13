addbufspc(int need)
{
    need *= 2;   /* for metafication */
    if((bv->bp - bv->buf) + need > bv->bufspc) {
	int bo = bv->bp - bv->buf;
	int bo1 = bv->bp1 ? bv->bp1 - bv->buf : -1;
	ptrdiff_t bufline_off = bv->bufline ? bv->bufline - bv->buf : -1;

	if(need & 255)
	    need = (need | 255) + 1;
	bv->buf = realloc(bv->buf, bv->bufspc += need);
	memset(bv->buf + bv->bufspc - need, 0, need);
	bv->bp = bv->buf + bo;
	if(bo1 != -1)
	    bv->bp1 = bv->buf + bo1;
	if (bufline_off != -1)
	    bv->bufline = bv->buf + bufline_off;
    }
}