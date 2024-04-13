decode_rfc3397(char *out, ssize_t len, int pl, const uint8_t *p)
{
 const uint8_t *r, *q = p;
 int count = 0, l, hops;
 uint8_t ltype;

 while (q - p < pl) {
		r = NULL;
		hops = 0;
 /* We check we are inside our length again incase
		 * the data is NOT terminated correctly. */
 while ((l = *q++) && q - p < pl) {
			ltype = l & 0xc0;
 if (ltype == 0x80 || ltype == 0x40)
 return 0;
 else if (ltype == 0xc0) { /* pointer */
				l = (l & 0x3f) << 8;
				l |= *q++;
 /* save source of first jump. */
 if (!r)
					r = q;
				hops++;
 if (hops > 255)
 return 0;
				q = p + l;
 if (q - p >= pl)
 return 0;
 } else {
 /* straightforward name segment, add with '.' */
				count += l + 1;
 if (out) {
 if ((ssize_t)l + 1 > len) {
						errno = ENOBUFS;
 return -1;
 }
					memcpy(out, q, l);
					out += l;
 *out++ = '.';
					len -= l;
					len--;
 }
				q += l;
 }
 }
 /* change last dot to space */
 if (out)
 *(out - 1) = ' ';
 if (r)
			q = r;
 }

 /* change last space to zero terminator */
 if (out)
 *(out - 1) = 0;

 return count; 
}
