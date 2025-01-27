nfsreply_print_noaddr(netdissect_options *ndo,
                      register const u_char *bp, u_int length,
                      register const u_char *bp2)
{
	register const struct sunrpc_msg *rp;
	uint32_t proc, vers, reply_stat;
	enum sunrpc_reject_stat rstat;
	uint32_t rlow;
	uint32_t rhigh;
	enum sunrpc_auth_stat rwhy;

	nfserr = 0;		/* assume no error */
	rp = (const struct sunrpc_msg *)bp;

	ND_TCHECK(rp->rm_reply.rp_stat);
	reply_stat = EXTRACT_32BITS(&rp->rm_reply.rp_stat);
	switch (reply_stat) {

	case SUNRPC_MSG_ACCEPTED:
		ND_PRINT((ndo, "reply ok %u", length));
		if (xid_map_find(rp, bp2, &proc, &vers) >= 0)
			interp_reply(ndo, rp, proc, vers, length);
		break;

	case SUNRPC_MSG_DENIED:
		ND_PRINT((ndo, "reply ERR %u: ", length));
		ND_TCHECK(rp->rm_reply.rp_reject.rj_stat);
		rstat = EXTRACT_32BITS(&rp->rm_reply.rp_reject.rj_stat);
		switch (rstat) {

		case SUNRPC_RPC_MISMATCH:
			ND_TCHECK(rp->rm_reply.rp_reject.rj_vers.high);
			rlow = EXTRACT_32BITS(&rp->rm_reply.rp_reject.rj_vers.low);
			rhigh = EXTRACT_32BITS(&rp->rm_reply.rp_reject.rj_vers.high);
			ND_PRINT((ndo, "RPC Version mismatch (%u-%u)", rlow, rhigh));
			break;

		case SUNRPC_AUTH_ERROR:
			ND_TCHECK(rp->rm_reply.rp_reject.rj_why);
			rwhy = EXTRACT_32BITS(&rp->rm_reply.rp_reject.rj_why);
			ND_PRINT((ndo, "Auth %s", tok2str(sunrpc_auth_str, "Invalid failure code %u", rwhy)));
			break;

		default:
			ND_PRINT((ndo, "Unknown reason for rejecting rpc message %u", (unsigned int)rstat));
			break;
		}
		break;

	default:
		ND_PRINT((ndo, "reply Unknown rpc response code=%u %u", reply_stat, length));
		break;
	}
	return;

trunc:
	if (!nfserr)
		ND_PRINT((ndo, "%s", tstr));
}
