int x25_parse_facilities(struct sk_buff *skb, struct x25_facilities *facilities,
		struct x25_dte_facilities *dte_facs, unsigned long *vc_fac_mask)
{
	unsigned char *p;
	unsigned int len;

	*vc_fac_mask = 0;

	/*
	 * The kernel knows which facilities were set on an incoming call but
	 * currently this information is not available to userspace.  Here we
	 * give userspace who read incoming call facilities 0 length to indicate
	 * it wasn't set.
	 */
	dte_facs->calling_len = 0;
	dte_facs->called_len = 0;
	memset(dte_facs->called_ae, '\0', sizeof(dte_facs->called_ae));
	memset(dte_facs->calling_ae, '\0', sizeof(dte_facs->calling_ae));

	if (!pskb_may_pull(skb, 1))
		return 0;

	len = skb->data[0];

	if (!pskb_may_pull(skb, 1 + len))
		return -1;

	p = skb->data + 1;

	while (len > 0) {
		switch (*p & X25_FAC_CLASS_MASK) {
		case X25_FAC_CLASS_A:
			if (len < 2)
				return -1;
			switch (*p) {
			case X25_FAC_REVERSE:
				if((p[1] & 0x81) == 0x81) {
					facilities->reverse = p[1] & 0x81;
					*vc_fac_mask |= X25_MASK_REVERSE;
					break;
				}

				if((p[1] & 0x01) == 0x01) {
					facilities->reverse = p[1] & 0x01;
					*vc_fac_mask |= X25_MASK_REVERSE;
					break;
				}

				if((p[1] & 0x80) == 0x80) {
					facilities->reverse = p[1] & 0x80;
					*vc_fac_mask |= X25_MASK_REVERSE;
					break;
				}

				if(p[1] == 0x00) {
					facilities->reverse
						= X25_DEFAULT_REVERSE;
					*vc_fac_mask |= X25_MASK_REVERSE;
					break;
				}

			case X25_FAC_THROUGHPUT:
				facilities->throughput = p[1];
				*vc_fac_mask |= X25_MASK_THROUGHPUT;
				break;
			case X25_MARKER:
				break;
			default:
				pr_debug("unknown facility "
				       "%02X, value %02X\n",
				       p[0], p[1]);
				break;
			}
			p   += 2;
			len -= 2;
			break;
		case X25_FAC_CLASS_B:
			if (len < 3)
				return -1;
			switch (*p) {
			case X25_FAC_PACKET_SIZE:
				facilities->pacsize_in  = p[1];
				facilities->pacsize_out = p[2];
				*vc_fac_mask |= X25_MASK_PACKET_SIZE;
				break;
			case X25_FAC_WINDOW_SIZE:
				facilities->winsize_in  = p[1];
				facilities->winsize_out = p[2];
				*vc_fac_mask |= X25_MASK_WINDOW_SIZE;
				break;
			default:
				pr_debug("unknown facility "
				       "%02X, values %02X, %02X\n",
				       p[0], p[1], p[2]);
				break;
			}
			p   += 3;
			len -= 3;
			break;
		case X25_FAC_CLASS_C:
			if (len < 4)
				return -1;
			pr_debug("unknown facility %02X, "
			       "values %02X, %02X, %02X\n",
			       p[0], p[1], p[2], p[3]);
			p   += 4;
			len -= 4;
			break;
		case X25_FAC_CLASS_D:
			if (len < p[1] + 2)
				return -1;
			switch (*p) {
			case X25_FAC_CALLING_AE:
				if (p[1] > X25_MAX_DTE_FACIL_LEN || p[1] <= 1)
					return -1;
				if (p[2] > X25_MAX_AE_LEN)
					return -1;
				dte_facs->calling_len = p[2];
				memcpy(dte_facs->calling_ae, &p[3], p[1] - 1);
				*vc_fac_mask |= X25_MASK_CALLING_AE;
				break;
			case X25_FAC_CALLED_AE:
				if (p[1] > X25_MAX_DTE_FACIL_LEN || p[1] <= 1)
					return -1;
				if (p[2] > X25_MAX_AE_LEN)
					return -1;
				dte_facs->called_len = p[2];
				memcpy(dte_facs->called_ae, &p[3], p[1] - 1);
				*vc_fac_mask |= X25_MASK_CALLED_AE;
				break;
			default:
				pr_debug("unknown facility %02X,"
					"length %d\n", p[0], p[1]);
				break;
			}
			len -= p[1] + 2;
			p += p[1] + 2;
			break;
		}
	}

	return p - skb->data;
}
