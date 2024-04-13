int rose_parse_facilities(unsigned char *p,
	struct rose_facilities_struct *facilities)
{
	int facilities_len, len;

	facilities_len = *p++;

	if (facilities_len == 0)
		return 0;

	while (facilities_len > 0) {
		if (*p == 0x00) {
			facilities_len--;
			p++;

 			switch (*p) {
 			case FAC_NATIONAL:		/* National */
 				len = rose_parse_national(p + 1, facilities, facilities_len - 1);
				if (len < 0)
					return 0;
 				facilities_len -= len + 1;
 				p += len + 1;
 				break;
 
 			case FAC_CCITT:		/* CCITT */
 				len = rose_parse_ccitt(p + 1, facilities, facilities_len - 1);
				if (len < 0)
					return 0;
 				facilities_len -= len + 1;
 				p += len + 1;
 				break;

			default:
				printk(KERN_DEBUG "ROSE: rose_parse_facilities - unknown facilities family %02X\n", *p);
				facilities_len--;
				p++;
				break;
			}
		} else
			break;	/* Error in facilities format */
	}

	return 1;
}