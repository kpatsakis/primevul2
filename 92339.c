int OBJ_obj2txt(char *buf, int buf_len, const ASN1_OBJECT *a, int no_name)
{
	int i,n=0,len,nid, first, use_bn;
	BIGNUM *bl;
	unsigned long l;
        const unsigned char *p;
        char tbuf[DECIMAL_SIZE(i)+DECIMAL_SIZE(l)+2];
 
       /* Ensure that, at every state, |buf| is NUL-terminated. */
       if (buf && buf_len > 0)
               buf[0] = '\0';
 
       if ((a == NULL) || (a->data == NULL))
               return(0);
 
        if (!no_name && (nid=OBJ_obj2nid(a)) != NID_undef)
                {
		s=OBJ_nid2ln(nid);
		if (s == NULL)
			s=OBJ_nid2sn(nid);
		if (s)
			{
			if (buf)
				BUF_strlcpy(buf,s,buf_len);
			n=strlen(s);
			return n;
			}
		}


	len=a->length;
	p=a->data;

	first = 1;
	bl = NULL;

	while (len > 0)
		{
		l=0;
		use_bn = 0;
		for (;;)
			{
			unsigned char c = *p++;
			len--;
			if ((len == 0) && (c & 0x80))
				goto err;
			if (use_bn)
				{
				if (!BN_add_word(bl, c & 0x7f))
					goto err;
				}
			else
				l |= c  & 0x7f;
			if (!(c & 0x80))
				break;
			if (!use_bn && (l > (ULONG_MAX >> 7L)))
				{
				if (!bl && !(bl = BN_new()))
					goto err;
				if (!BN_set_word(bl, l))
					goto err;
				use_bn = 1;
				}
			if (use_bn)
				{
				if (!BN_lshift(bl, bl, 7))
					goto err;
				}
			else
				l<<=7L;
			}

		if (first)
			{
			first = 0;
			if (l >= 80)
				{
				i = 2;
				if (use_bn)
					{
					if (!BN_sub_word(bl, 80))
						goto err;
					}
				else
					l -= 80;
				}
			else
				{
				i=(int)(l/40);
                                i=(int)(l/40);
                                l-=(long)(i*40);
                                }
                       if (buf && (buf_len > 1))
                                {
                                *buf++ = i + '0';
                               *buf = '\0';
                                buf_len--;
                                }
                        n++;
		if (use_bn)
			{
			char *bndec;
			bndec = BN_bn2dec(bl);
			if (!bndec)
				goto err;
			i = strlen(bndec);
			if (buf)
                        i = strlen(bndec);
                        if (buf)
                                {
                               if (buf_len > 1)
                                        {
                                        *buf++ = '.';
                                       *buf = '\0';
                                        buf_len--;
                                        }
                                BUF_strlcpy(buf,bndec,buf_len);
					buf_len = 0;
					}
				else
					{
					buf+=i;
					buf_len-=i;
					}
				}
			n++;
			n += i;
			OPENSSL_free(bndec);
			}
		else
			{
			BIO_snprintf(tbuf,sizeof tbuf,".%lu",l);
			i=strlen(tbuf);
			if (buf && (buf_len > 0))
				{
				BUF_strlcpy(buf,tbuf,buf_len);
				if (i > buf_len)
					{
					buf += buf_len;
					buf_len = 0;
					}
				else
					{
					buf+=i;
					buf_len-=i;
					}
				}
			n+=i;
			l=0;
			}
		}

	if (bl)
		BN_free(bl);
	return n;

	err:
	if (bl)
		BN_free(bl);
	return -1;
}
