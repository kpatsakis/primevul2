pputc(char c)
{
    if (imeta(c)) {
	*bv->bp++ = Meta;
	c ^= 32;
    }
    *bv->bp++ = c;
    if (c == '\n' && !bv->dontcount)
	bv->bufline = bv->bp;
}