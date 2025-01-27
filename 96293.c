static void receive_udppacket(node_t *n, vpn_packet_t *inpkt) {
	vpn_packet_t pkt1, pkt2;
	vpn_packet_t *pkt[] = { &pkt1, &pkt2, &pkt1, &pkt2 };
	int nextpkt = 0;
	vpn_packet_t *outpkt = pkt[0];
	int outlen, outpad;
	unsigned char hmac[EVP_MAX_MD_SIZE];
	int i;

	if(!n->inkey) {
		ifdebug(TRAFFIC) logger(LOG_DEBUG, "Got packet from %s (%s) but he hasn't got our key yet",
					n->name, n->hostname);
		return;
	}

	/* Check packet length */

	if(inpkt->len < sizeof(inpkt->seqno) + n->inmaclength) {
		ifdebug(TRAFFIC) logger(LOG_DEBUG, "Got too short packet from %s (%s)",
					n->name, n->hostname);
		return;
	}

	/* Check the message authentication code */

	if(n->indigest && n->inmaclength) {
		inpkt->len -= n->inmaclength;
		HMAC(n->indigest, n->inkey, n->inkeylength,
			 (unsigned char *) &inpkt->seqno, inpkt->len, (unsigned char *)hmac, NULL);

		if(memcmp(hmac, (char *) &inpkt->seqno + inpkt->len, n->inmaclength)) {
			ifdebug(TRAFFIC) logger(LOG_DEBUG, "Got unauthenticated packet from %s (%s)",
					   n->name, n->hostname);
			return;
		}
	}

	/* Decrypt the packet */

	if(n->incipher) {
		outpkt = pkt[nextpkt++];

		if(!EVP_DecryptInit_ex(&n->inctx, NULL, NULL, NULL, NULL)
				|| !EVP_DecryptUpdate(&n->inctx, (unsigned char *) &outpkt->seqno, &outlen,
					(unsigned char *) &inpkt->seqno, inpkt->len)
				|| !EVP_DecryptFinal_ex(&n->inctx, (unsigned char *) &outpkt->seqno + outlen, &outpad)) {
			ifdebug(TRAFFIC) logger(LOG_DEBUG, "Error decrypting packet from %s (%s): %s",
						n->name, n->hostname, ERR_error_string(ERR_get_error(), NULL));
			return;
		}
		
		outpkt->len = outlen + outpad;
		inpkt = outpkt;
	}

	/* Check the sequence number */

	inpkt->len -= sizeof(inpkt->seqno);
	inpkt->seqno = ntohl(inpkt->seqno);

	if(replaywin) {
		if(inpkt->seqno != n->received_seqno + 1) {
			if(inpkt->seqno >= n->received_seqno + replaywin * 8) {
				if(n->farfuture++ < replaywin >> 2) {
					logger(LOG_WARNING, "Packet from %s (%s) is %d seqs in the future, dropped (%u)",
						n->name, n->hostname, inpkt->seqno - n->received_seqno - 1, n->farfuture);
					return;
				}
				logger(LOG_WARNING, "Lost %d packets from %s (%s)",
					   	inpkt->seqno - n->received_seqno - 1, n->name, n->hostname);
				memset(n->late, 0, replaywin);
			} else if (inpkt->seqno <= n->received_seqno) {
				if((n->received_seqno >= replaywin * 8 && inpkt->seqno <= n->received_seqno - replaywin * 8) || !(n->late[(inpkt->seqno / 8) % replaywin] & (1 << inpkt->seqno % 8))) {
					logger(LOG_WARNING, "Got late or replayed packet from %s (%s), seqno %d, last received %d",
					   	n->name, n->hostname, inpkt->seqno, n->received_seqno);
					return;
				}
			} else {
				for(i = n->received_seqno + 1; i < inpkt->seqno; i++)
					n->late[(i / 8) % replaywin] |= 1 << i % 8;
			}
		}

		n->farfuture = 0;
		n->late[(inpkt->seqno / 8) % replaywin] &= ~(1 << inpkt->seqno % 8);
	}

	if(inpkt->seqno > n->received_seqno)
		n->received_seqno = inpkt->seqno;
			
	if(n->received_seqno > MAX_SEQNO)
		keyexpires = 0;

	/* Decompress the packet */

	length_t origlen = inpkt->len;

	if(n->incompression) {
		outpkt = pkt[nextpkt++];

		if((outpkt->len = uncompress_packet(outpkt->data, inpkt->data, inpkt->len, n->incompression)) < 0) {
			ifdebug(TRAFFIC) logger(LOG_ERR, "Error while uncompressing packet from %s (%s)",
				  		 n->name, n->hostname);
			return;
		}

		inpkt = outpkt;

		origlen -= MTU/64 + 20;
	}

	inpkt->priority = 0;

	if(!inpkt->data[12] && !inpkt->data[13])
		mtu_probe_h(n, inpkt, origlen);
	else
		receive_packet(n, inpkt);
}
