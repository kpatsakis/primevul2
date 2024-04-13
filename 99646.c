_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
	char name[256];
	struct FAB fab;
	unsigned short channel;
	char *inadr[2], *retadr[2];
	unsigned long status;
	long size;
	
	if (no_mapped >= MAX_MAPPED)
		return(0);
	/*
	 * We cannot use a file descriptor, we
	 * must open the file once more.
	 */
	if (getname((int)fd, name, 1) == NULL)
		return(0);
	/* prepare the FAB for a user file open */
	fab = cc$rms_fab;
	fab.fab$l_fop |= FAB$V_UFO;
	fab.fab$b_fac = FAB$M_GET;
	fab.fab$b_shr = FAB$M_SHRGET;
	fab.fab$l_fna = name;
	fab.fab$b_fns = strlen(name);
	status = sys$open(&fab);	/* open file & get channel number */
	if ((status&1) == 0)
		return(0);
	channel = (unsigned short)fab.fab$l_stv;
	inadr[0] = inadr[1] = (char *)0; /* just an address in P0 space */
	/*
	 * Map the blocks of the file up to
	 * the EOF block into virtual memory.
	 */
	size = _tiffSizeProc(fd);
	status = sys$crmpsc(inadr, retadr, 0, SEC$M_EXPREG, 0,0,0, channel,
		TIFFhowmany(size,512), 0,0,0);  ddd
	if ((status&1) == 0){
		sys$dassgn(channel);
		return(0);
	}
	*pbase = (tdata_t) retadr[0];	/* starting virtual address */
	/*
	 * Use the size of the file up to the
	 * EOF mark for UNIX compatibility.
	 */
	*psize = (toff_t) size;
	/* Record the section in the table */
	map_table[no_mapped].base = retadr[0];
	map_table[no_mapped].top = retadr[1];
	map_table[no_mapped].channel = channel;
	no_mapped++;

        return(1);
}
