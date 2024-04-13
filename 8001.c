static void route4_bind_class(void *fh, u32 classid, unsigned long cl, void *q,
			      unsigned long base)
{
	struct route4_filter *f = fh;

	if (f && f->res.classid == classid) {
		if (cl)
			__tcf_bind_filter(q, &f->res, base);
		else
			__tcf_unbind_filter(q, &f->res);
	}
}