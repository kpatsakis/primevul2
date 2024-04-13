int ssl3_send_server_key_exchange(SSL *s)
	{
#ifndef OPENSSL_NO_RSA
	unsigned char *q;
	int j,num;
	RSA *rsa;
	unsigned char md_buf[MD5_DIGEST_LENGTH+SHA_DIGEST_LENGTH];
	unsigned int u;
#endif
#ifndef OPENSSL_NO_DH
	DH *dh=NULL,*dhp;
#endif
#ifndef OPENSSL_NO_ECDH
	EC_KEY *ecdh=NULL, *ecdhp;
	unsigned char *encodedPoint = NULL;
	int encodedlen = 0;
	int curve_id = 0;
	BN_CTX *bn_ctx = NULL; 
#endif
	EVP_PKEY *pkey;
	const EVP_MD *md = NULL;
	unsigned char *p,*d;
	int al,i;
	unsigned long type;
	int n;
	CERT *cert;
	BIGNUM *r[4];
	int nr[4],kn;
	BUF_MEM *buf;
	EVP_MD_CTX md_ctx;

	EVP_MD_CTX_init(&md_ctx);
	if (s->state == SSL3_ST_SW_KEY_EXCH_A)
		{
		type=s->s3->tmp.new_cipher->algorithm_mkey;
		cert=s->cert;

		buf=s->init_buf;

		r[0]=r[1]=r[2]=r[3]=NULL;
		n=0;
#ifndef OPENSSL_NO_RSA
		if (type & SSL_kRSA)
			{
			rsa=cert->rsa_tmp;
			if ((rsa == NULL) && (s->cert->rsa_tmp_cb != NULL))
				{
				rsa=s->cert->rsa_tmp_cb(s,
				      SSL_C_IS_EXPORT(s->s3->tmp.new_cipher),
				      SSL_C_EXPORT_PKEYLENGTH(s->s3->tmp.new_cipher));
				if(rsa == NULL)
				{
					al=SSL_AD_HANDSHAKE_FAILURE;
					SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_ERROR_GENERATING_TMP_RSA_KEY);
					goto f_err;
				}
				RSA_up_ref(rsa);
				cert->rsa_tmp=rsa;
				}
			if (rsa == NULL)
				{
				al=SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_MISSING_TMP_RSA_KEY);
				goto f_err;
				}
			r[0]=rsa->n;
			r[1]=rsa->e;
			s->s3->tmp.use_rsa_tmp=1;
			}
		else
#endif
#ifndef OPENSSL_NO_DH
			if (type & SSL_kDHE)
			{
			if (s->cert->dh_tmp_auto)
				{
				dhp = ssl_get_auto_dh(s);
				if (dhp == NULL)
					{
					al=SSL_AD_INTERNAL_ERROR;
					SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_INTERNAL_ERROR);
					goto f_err;
					}
				}
			else
				dhp=cert->dh_tmp;
			if ((dhp == NULL) && (s->cert->dh_tmp_cb != NULL))
				dhp=s->cert->dh_tmp_cb(s,
				      SSL_C_IS_EXPORT(s->s3->tmp.new_cipher),
				      SSL_C_EXPORT_PKEYLENGTH(s->s3->tmp.new_cipher));
			if (dhp == NULL)
				{
				al=SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_MISSING_TMP_DH_KEY);
				goto f_err;
				}
			if (!ssl_security(s, SSL_SECOP_TMP_DH,
						DH_security_bits(dhp), 0, dhp))
				{
				al=SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_DH_KEY_TOO_SMALL);
				goto f_err;
				}
			if (s->s3->tmp.dh != NULL)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE, ERR_R_INTERNAL_ERROR);
				goto err;
				}

			if (s->cert->dh_tmp_auto)
				dh = dhp;
			else if ((dh=DHparams_dup(dhp)) == NULL)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_DH_LIB);
				goto err;
				}

			s->s3->tmp.dh=dh;
			if ((dhp->pub_key == NULL ||
			     dhp->priv_key == NULL ||
			     (s->options & SSL_OP_SINGLE_DH_USE)))
				{
				if(!DH_generate_key(dh))
				    {
				    SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,
					   ERR_R_DH_LIB);
				    goto err;
				    }
				}
			else
				{
				dh->pub_key=BN_dup(dhp->pub_key);
				dh->priv_key=BN_dup(dhp->priv_key);
				if ((dh->pub_key == NULL) ||
					(dh->priv_key == NULL))
					{
					SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_DH_LIB);
					goto err;
					}
				}
			r[0]=dh->p;
			r[1]=dh->g;
			r[2]=dh->pub_key;
			}
		else 
#endif
#ifndef OPENSSL_NO_ECDH
			if (type & SSL_kECDHE)
			{
			const EC_GROUP *group;

			ecdhp=cert->ecdh_tmp;
			if (s->cert->ecdh_tmp_auto)
				{
				/* Get NID of appropriate shared curve */
				int nid = tls1_shared_curve(s, -2);
				if (nid != NID_undef)
					ecdhp = EC_KEY_new_by_curve_name(nid);
				}
			else if ((ecdhp == NULL) && s->cert->ecdh_tmp_cb)
				{
				ecdhp=s->cert->ecdh_tmp_cb(s,
				      SSL_C_IS_EXPORT(s->s3->tmp.new_cipher),
				      SSL_C_EXPORT_PKEYLENGTH(s->s3->tmp.new_cipher));
				}
			if (ecdhp == NULL)
				{
				al=SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_MISSING_TMP_ECDH_KEY);
				goto f_err;
				}

			if (s->s3->tmp.ecdh != NULL)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE, ERR_R_INTERNAL_ERROR);
				goto err;
				}

			/* Duplicate the ECDH structure. */
			if (ecdhp == NULL)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_ECDH_LIB);
				goto err;
				}
			if (s->cert->ecdh_tmp_auto)
				ecdh = ecdhp;
			else if ((ecdh = EC_KEY_dup(ecdhp)) == NULL)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_ECDH_LIB);
				goto err;
				}

			s->s3->tmp.ecdh=ecdh;
			if ((EC_KEY_get0_public_key(ecdh) == NULL) ||
			    (EC_KEY_get0_private_key(ecdh) == NULL) ||
			    (s->options & SSL_OP_SINGLE_ECDH_USE))
				{
				if(!EC_KEY_generate_key(ecdh))
				    {
				    SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_ECDH_LIB);
				    goto err;
				    }
				}

			if (((group = EC_KEY_get0_group(ecdh)) == NULL) ||
			    (EC_KEY_get0_public_key(ecdh)  == NULL) ||
			    (EC_KEY_get0_private_key(ecdh) == NULL))
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_ECDH_LIB);
				goto err;
				}

			if (SSL_C_IS_EXPORT(s->s3->tmp.new_cipher) &&
			    (EC_GROUP_get_degree(group) > 163)) 
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_ECGROUP_TOO_LARGE_FOR_CIPHER);
				goto err;
				}

			/* XXX: For now, we only support ephemeral ECDH
			 * keys over named (not generic) curves. For 
			 * supported named curves, curve_id is non-zero.
			 */
			if ((curve_id = 
			    tls1_ec_nid2curve_id(EC_GROUP_get_curve_name(group)))
			    == 0)
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_UNSUPPORTED_ELLIPTIC_CURVE);
				goto err;
				}

			/* Encode the public key.
			 * First check the size of encoding and
			 * allocate memory accordingly.
			 */
			encodedlen = EC_POINT_point2oct(group, 
			    EC_KEY_get0_public_key(ecdh),
			    POINT_CONVERSION_UNCOMPRESSED, 
			    NULL, 0, NULL);

			encodedPoint = (unsigned char *) 
			    OPENSSL_malloc(encodedlen*sizeof(unsigned char)); 
			bn_ctx = BN_CTX_new();
			if ((encodedPoint == NULL) || (bn_ctx == NULL))
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_MALLOC_FAILURE);
				goto err;
				}


			encodedlen = EC_POINT_point2oct(group, 
			    EC_KEY_get0_public_key(ecdh), 
			    POINT_CONVERSION_UNCOMPRESSED, 
			    encodedPoint, encodedlen, bn_ctx);

			if (encodedlen == 0) 
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_ECDH_LIB);
				goto err;
				}

			BN_CTX_free(bn_ctx);  bn_ctx=NULL;

			/* XXX: For now, we only support named (not 
			 * generic) curves in ECDH ephemeral key exchanges.
			 * In this situation, we need four additional bytes
			 * to encode the entire ServerECDHParams
			 * structure. 
			 */
			n = 4 + encodedlen;

			/* We'll generate the serverKeyExchange message
			 * explicitly so we can set these to NULLs
			 */
			r[0]=NULL;
			r[1]=NULL;
			r[2]=NULL;
			r[3]=NULL;
			}
		else 
#endif /* !OPENSSL_NO_ECDH */
#ifndef OPENSSL_NO_PSK
			if (type & SSL_kPSK)
				{
				/* reserve size for record length and PSK identity hint*/
				n+=2+strlen(s->ctx->psk_identity_hint);
				}
			else
#endif /* !OPENSSL_NO_PSK */
#ifndef OPENSSL_NO_SRP
		if (type & SSL_kSRP)
			{
			if ((s->srp_ctx.N == NULL) ||
				(s->srp_ctx.g == NULL) ||
				(s->srp_ctx.s == NULL) ||
				(s->srp_ctx.B == NULL))
				{
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_MISSING_SRP_PARAM);
				goto err;
				}
			r[0]=s->srp_ctx.N;
			r[1]=s->srp_ctx.g;
			r[2]=s->srp_ctx.s;
			r[3]=s->srp_ctx.B;
			}
		else 
#endif
			{
			al=SSL_AD_HANDSHAKE_FAILURE;
			SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_UNKNOWN_KEY_EXCHANGE_TYPE);
			goto f_err;
			}
		for (i=0; i < 4 && r[i] != NULL; i++)
			{
			nr[i]=BN_num_bytes(r[i]);
#ifndef OPENSSL_NO_SRP
			if ((i == 2) && (type & SSL_kSRP))
				n+=1+nr[i];
			else
#endif
			n+=2+nr[i];
			}

		if (!(s->s3->tmp.new_cipher->algorithm_auth & (SSL_aNULL|SSL_aSRP))
			&& !(s->s3->tmp.new_cipher->algorithm_mkey & SSL_kPSK))
			{
			if ((pkey=ssl_get_sign_pkey(s,s->s3->tmp.new_cipher,&md))
				== NULL)
				{
				al=SSL_AD_DECODE_ERROR;
				goto f_err;
				}
			kn=EVP_PKEY_size(pkey);
			}
		else
			{
			pkey=NULL;
			kn=0;
			}

		if (!BUF_MEM_grow_clean(buf,n+SSL_HM_HEADER_LENGTH(s)+kn))
			{
			SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_LIB_BUF);
			goto err;
			}
		d = p = ssl_handshake_start(s);

		for (i=0; i < 4 && r[i] != NULL; i++)
			{
#ifndef OPENSSL_NO_SRP
			if ((i == 2) && (type & SSL_kSRP))
				{
				*p = nr[i];
				p++;
				}
			else
#endif
			s2n(nr[i],p);
			BN_bn2bin(r[i],p);
			p+=nr[i];
			}

#ifndef OPENSSL_NO_ECDH
		if (type & SSL_kECDHE) 
			{
			/* XXX: For now, we only support named (not generic) curves.
			 * In this situation, the serverKeyExchange message has:
			 * [1 byte CurveType], [2 byte CurveName]
			 * [1 byte length of encoded point], followed by
			 * the actual encoded point itself
			 */
			*p = NAMED_CURVE_TYPE;
			p += 1;
			*p = 0;
			p += 1;
			*p = curve_id;
			p += 1;
			*p = encodedlen;
			p += 1;
			memcpy((unsigned char*)p, 
			    (unsigned char *)encodedPoint, 
			    encodedlen);
			OPENSSL_free(encodedPoint);
			encodedPoint = NULL;
			p += encodedlen;
			}
#endif

#ifndef OPENSSL_NO_PSK
		if (type & SSL_kPSK)
			{
			/* copy PSK identity hint */
			s2n(strlen(s->ctx->psk_identity_hint), p); 
			strncpy((char *)p, s->ctx->psk_identity_hint, strlen(s->ctx->psk_identity_hint));
			p+=strlen(s->ctx->psk_identity_hint);
			}
#endif

		/* not anonymous */
		if (pkey != NULL)
			{
			/* n is the length of the params, they start at &(d[4])
			 * and p points to the space at the end. */
#ifndef OPENSSL_NO_RSA
			if (pkey->type == EVP_PKEY_RSA && !SSL_USE_SIGALGS(s))
				{
				q=md_buf;
				j=0;
				for (num=2; num > 0; num--)
					{
					EVP_MD_CTX_set_flags(&md_ctx,
						EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
					EVP_DigestInit_ex(&md_ctx,(num == 2)
						?s->ctx->md5:s->ctx->sha1, NULL);
					EVP_DigestUpdate(&md_ctx,&(s->s3->client_random[0]),SSL3_RANDOM_SIZE);
					EVP_DigestUpdate(&md_ctx,&(s->s3->server_random[0]),SSL3_RANDOM_SIZE);
					EVP_DigestUpdate(&md_ctx,d,n);
					EVP_DigestFinal_ex(&md_ctx,q,
						(unsigned int *)&i);
					q+=i;
					j+=i;
					}
				if (RSA_sign(NID_md5_sha1, md_buf, j,
					&(p[2]), &u, pkey->pkey.rsa) <= 0)
					{
					SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_LIB_RSA);
					goto err;
					}
				s2n(u,p);
				n+=u+2;
				}
			else
#endif
			if (md)
				{
				/* send signature algorithm */
				if (SSL_USE_SIGALGS(s))
					{
					if (!tls12_get_sigandhash(p, pkey, md))
						{
						/* Should never happen */
						al=SSL_AD_INTERNAL_ERROR;
						SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_R_INTERNAL_ERROR);
						goto f_err;
						}
					p+=2;
					}
#ifdef SSL_DEBUG
				fprintf(stderr, "Using hash %s\n",
							EVP_MD_name(md));
#endif
				EVP_SignInit_ex(&md_ctx, md, NULL);
				EVP_SignUpdate(&md_ctx,&(s->s3->client_random[0]),SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx,&(s->s3->server_random[0]),SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx,d,n);
				if (!EVP_SignFinal(&md_ctx,&(p[2]),
					(unsigned int *)&i,pkey))
					{
					SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,ERR_LIB_EVP);
					goto err;
					}
				s2n(i,p);
				n+=i+2;
				if (SSL_USE_SIGALGS(s))
					n+= 2;
				}
			else
				{
				/* Is this error check actually needed? */
				al=SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,SSL_R_UNKNOWN_PKEY_TYPE);
				goto f_err;
				}
			}

		ssl_set_handshake_header(s, SSL3_MT_SERVER_KEY_EXCHANGE, n);
		}

	s->state = SSL3_ST_SW_KEY_EXCH_B;
	EVP_MD_CTX_cleanup(&md_ctx);
	return ssl_do_write(s);
f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
#ifndef OPENSSL_NO_ECDH
	if (encodedPoint != NULL) OPENSSL_free(encodedPoint);
	BN_CTX_free(bn_ctx);
#endif
	EVP_MD_CTX_cleanup(&md_ctx);
	return(-1);
	}
