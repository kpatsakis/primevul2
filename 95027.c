int http_process_res_common(struct stream *s, struct channel *rep, int an_bit, struct proxy *px)
{
	struct session *sess = s->sess;
	struct http_txn *txn = s->txn;
	struct http_msg *msg = &txn->rsp;
	struct proxy *cur_proxy;
	struct cond_wordlist *wl;
	enum rule_result ret = HTTP_RULE_RES_CONT;

	DPRINTF(stderr,"[%u] %s: stream=%p b=%p, exp(r,w)=%u,%u bf=%08x bh=%d analysers=%02x\n",
		now_ms, __FUNCTION__,
		s,
		rep,
		rep->rex, rep->wex,
		rep->flags,
		rep->buf->i,
		rep->analysers);

	if (unlikely(msg->msg_state < HTTP_MSG_BODY))	/* we need more data */
		return 0;

	/* The stats applet needs to adjust the Connection header but we don't
	 * apply any filter there.
	 */
	if (unlikely(objt_applet(s->target) == &http_stats_applet)) {
		rep->analysers &= ~an_bit;
		rep->analyse_exp = TICK_ETERNITY;
		goto skip_filters;
	}

	/*
	 * We will have to evaluate the filters.
	 * As opposed to version 1.2, now they will be evaluated in the
	 * filters order and not in the header order. This means that
	 * each filter has to be validated among all headers.
	 *
	 * Filters are tried with ->be first, then with ->fe if it is
	 * different from ->be.
	 *
	 * Maybe we are in resume condiion. In this case I choose the
	 * "struct proxy" which contains the rule list matching the resume
	 * pointer. If none of theses "struct proxy" match, I initialise
	 * the process with the first one.
	 *
	 * In fact, I check only correspondance betwwen the current list
	 * pointer and the ->fe rule list. If it doesn't match, I initialize
	 * the loop with the ->be.
	 */
	if (s->current_rule_list == &sess->fe->http_res_rules)
		cur_proxy = sess->fe;
	else
		cur_proxy = s->be;
	while (1) {
		struct proxy *rule_set = cur_proxy;

		/* evaluate http-response rules */
		if (ret == HTTP_RULE_RES_CONT) {
			ret = http_res_get_intercept_rule(cur_proxy, &cur_proxy->http_res_rules, s);

			if (ret == HTTP_RULE_RES_BADREQ)
				goto return_srv_prx_502;

			if (ret == HTTP_RULE_RES_DONE) {
				rep->analysers &= ~an_bit;
				rep->analyse_exp = TICK_ETERNITY;
				return 1;
			}
		}

		/* we need to be called again. */
		if (ret == HTTP_RULE_RES_YIELD) {
			channel_dont_close(rep);
			return 0;
		}

		/* try headers filters */
		if (rule_set->rsp_exp != NULL) {
			if (apply_filters_to_response(s, rep, rule_set) < 0) {
			return_bad_resp:
				if (objt_server(s->target)) {
					HA_ATOMIC_ADD(&objt_server(s->target)->counters.failed_resp, 1);
					health_adjust(objt_server(s->target), HANA_STATUS_HTTP_RSP);
				}
				HA_ATOMIC_ADD(&s->be->be_counters.failed_resp, 1);
			return_srv_prx_502:
				rep->analysers &= AN_RES_FLT_END;
				txn->status = 502;
				s->logs.t_data = -1; /* was not a valid response */
				s->si[1].flags |= SI_FL_NOLINGER;
				channel_truncate(rep);
				http_reply_and_close(s, txn->status, http_error_message(s));
				if (!(s->flags & SF_ERR_MASK))
					s->flags |= SF_ERR_PRXCOND;
				if (!(s->flags & SF_FINST_MASK))
					s->flags |= SF_FINST_H;
				return 0;
			}
		}

		/* has the response been denied ? */
		if (txn->flags & TX_SVDENY) {
			if (objt_server(s->target))
				HA_ATOMIC_ADD(&objt_server(s->target)->counters.failed_secu, 1);

			HA_ATOMIC_ADD(&s->be->be_counters.denied_resp, 1);
			HA_ATOMIC_ADD(&sess->fe->fe_counters.denied_resp, 1);
			if (sess->listener->counters)
				HA_ATOMIC_ADD(&sess->listener->counters->denied_resp, 1);

			goto return_srv_prx_502;
		}

		/* add response headers from the rule sets in the same order */
		list_for_each_entry(wl, &rule_set->rsp_add, list) {
			if (txn->status < 200 && txn->status != 101)
				break;
			if (wl->cond) {
				int ret = acl_exec_cond(wl->cond, px, sess, s, SMP_OPT_DIR_RES|SMP_OPT_FINAL);
				ret = acl_pass(ret);
				if (((struct acl_cond *)wl->cond)->pol == ACL_COND_UNLESS)
					ret = !ret;
				if (!ret)
					continue;
			}
			if (unlikely(http_header_add_tail(&txn->rsp, &txn->hdr_idx, wl->s) < 0))
				goto return_bad_resp;
		}

		/* check whether we're already working on the frontend */
		if (cur_proxy == sess->fe)
			break;
		cur_proxy = sess->fe;
	}

	/* After this point, this anayzer can't return yield, so we can
	 * remove the bit corresponding to this analyzer from the list.
	 *
	 * Note that the intermediate returns and goto found previously
	 * reset the analyzers.
	 */
	rep->analysers &= ~an_bit;
	rep->analyse_exp = TICK_ETERNITY;

	/* OK that's all we can do for 1xx responses */
	if (unlikely(txn->status < 200 && txn->status != 101))
		goto skip_header_mangling;

	/*
	 * Now check for a server cookie.
	 */
	if (s->be->cookie_name || sess->fe->capture_name || (s->be->options & PR_O_CHK_CACHE))
		manage_server_side_cookies(s, rep);

	/*
	 * Check for cache-control or pragma headers if required.
	 */
	if ((s->be->options & PR_O_CHK_CACHE) || (s->be->ck_opts & PR_CK_NOC))
		check_response_for_cacheability(s, rep);

	/*
	 * Add server cookie in the response if needed
	 */
	if (objt_server(s->target) && (s->be->ck_opts & PR_CK_INS) &&
	    !((txn->flags & TX_SCK_FOUND) && (s->be->ck_opts & PR_CK_PSV)) &&
	    (!(s->flags & SF_DIRECT) ||
	     ((s->be->cookie_maxidle || txn->cookie_last_date) &&
	      (!txn->cookie_last_date || (txn->cookie_last_date - date.tv_sec) < 0)) ||
	     (s->be->cookie_maxlife && !txn->cookie_first_date) ||  // set the first_date
	     (!s->be->cookie_maxlife && txn->cookie_first_date)) && // remove the first_date
	    (!(s->be->ck_opts & PR_CK_POST) || (txn->meth == HTTP_METH_POST)) &&
	    !(s->flags & SF_IGNORE_PRST)) {
		/* the server is known, it's not the one the client requested, or the
		 * cookie's last seen date needs to be refreshed. We have to
		 * insert a set-cookie here, except if we want to insert only on POST
		 * requests and this one isn't. Note that servers which don't have cookies
		 * (eg: some backup servers) will return a full cookie removal request.
		 */
		if (!objt_server(s->target)->cookie) {
			chunk_printf(&trash,
				     "Set-Cookie: %s=; Expires=Thu, 01-Jan-1970 00:00:01 GMT; path=/",
				     s->be->cookie_name);
		}
		else {
			chunk_printf(&trash, "Set-Cookie: %s=%s", s->be->cookie_name, objt_server(s->target)->cookie);

			if (s->be->cookie_maxidle || s->be->cookie_maxlife) {
				/* emit last_date, which is mandatory */
				trash.str[trash.len++] = COOKIE_DELIM_DATE;
				s30tob64((date.tv_sec+3) >> 2, trash.str + trash.len);
				trash.len += 5;

				if (s->be->cookie_maxlife) {
					/* emit first_date, which is either the original one or
					 * the current date.
					 */
					trash.str[trash.len++] = COOKIE_DELIM_DATE;
					s30tob64(txn->cookie_first_date ?
						 txn->cookie_first_date >> 2 :
						 (date.tv_sec+3) >> 2, trash.str + trash.len);
					trash.len += 5;
				}
			}
			chunk_appendf(&trash, "; path=/");
		}

		if (s->be->cookie_domain)
			chunk_appendf(&trash, "; domain=%s", s->be->cookie_domain);

		if (s->be->ck_opts & PR_CK_HTTPONLY)
			chunk_appendf(&trash, "; HttpOnly");

		if (s->be->ck_opts & PR_CK_SECURE)
			chunk_appendf(&trash, "; Secure");

		if (unlikely(http_header_add_tail2(&txn->rsp, &txn->hdr_idx, trash.str, trash.len) < 0))
			goto return_bad_resp;

		txn->flags &= ~TX_SCK_MASK;
		if (objt_server(s->target)->cookie && (s->flags & SF_DIRECT))
			/* the server did not change, only the date was updated */
			txn->flags |= TX_SCK_UPDATED;
		else
			txn->flags |= TX_SCK_INSERTED;

		/* Here, we will tell an eventual cache on the client side that we don't
		 * want it to cache this reply because HTTP/1.0 caches also cache cookies !
		 * Some caches understand the correct form: 'no-cache="set-cookie"', but
		 * others don't (eg: apache <= 1.3.26). So we use 'private' instead.
		 */
		if ((s->be->ck_opts & PR_CK_NOC) && (txn->flags & TX_CACHEABLE)) {

			txn->flags &= ~TX_CACHEABLE & ~TX_CACHE_COOK;

			if (unlikely(http_header_add_tail2(&txn->rsp, &txn->hdr_idx,
			                                   "Cache-control: private", 22) < 0))
				goto return_bad_resp;
		}
	}

	/*
	 * Check if result will be cacheable with a cookie.
	 * We'll block the response if security checks have caught
	 * nasty things such as a cacheable cookie.
	 */
	if (((txn->flags & (TX_CACHEABLE | TX_CACHE_COOK | TX_SCK_PRESENT)) ==
	     (TX_CACHEABLE | TX_CACHE_COOK | TX_SCK_PRESENT)) &&
	    (s->be->options & PR_O_CHK_CACHE)) {
		/* we're in presence of a cacheable response containing
		 * a set-cookie header. We'll block it as requested by
		 * the 'checkcache' option, and send an alert.
		 */
		if (objt_server(s->target))
			HA_ATOMIC_ADD(&objt_server(s->target)->counters.failed_secu, 1);

		HA_ATOMIC_ADD(&s->be->be_counters.denied_resp, 1);
		HA_ATOMIC_ADD(&sess->fe->fe_counters.denied_resp, 1);
		if (sess->listener->counters)
			HA_ATOMIC_ADD(&sess->listener->counters->denied_resp, 1);

		ha_alert("Blocking cacheable cookie in response from instance %s, server %s.\n",
			 s->be->id, objt_server(s->target) ? objt_server(s->target)->id : "<dispatch>");
		send_log(s->be, LOG_ALERT,
			 "Blocking cacheable cookie in response from instance %s, server %s.\n",
			 s->be->id, objt_server(s->target) ? objt_server(s->target)->id : "<dispatch>");
		goto return_srv_prx_502;
	}

 skip_filters:
	/*
	 * Adjust "Connection: close" or "Connection: keep-alive" if needed.
	 * If an "Upgrade" token is found, the header is left untouched in order
	 * not to have to deal with some client bugs : some of them fail an upgrade
	 * if anything but "Upgrade" is present in the Connection header. We don't
	 * want to touch any 101 response either since it's switching to another
	 * protocol.
	 */
	if ((txn->status != 101) && !(txn->flags & TX_HDR_CONN_UPG) &&
	    (((txn->flags & TX_CON_WANT_MSK) != TX_CON_WANT_TUN) ||
	     ((sess->fe->options & PR_O_HTTP_MODE) == PR_O_HTTP_PCL ||
	      (s->be->options & PR_O_HTTP_MODE) == PR_O_HTTP_PCL))) {
		unsigned int want_flags = 0;

		if ((txn->flags & TX_CON_WANT_MSK) == TX_CON_WANT_KAL ||
		    (txn->flags & TX_CON_WANT_MSK) == TX_CON_WANT_SCL) {
			/* we want a keep-alive response here. Keep-alive header
			 * required if either side is not 1.1.
			 */
			if (!(txn->req.flags & msg->flags & HTTP_MSGF_VER_11))
				want_flags |= TX_CON_KAL_SET;
		}
		else {
			/* we want a close response here. Close header required if
			 * the server is 1.1, regardless of the client.
			 */
			if (msg->flags & HTTP_MSGF_VER_11)
				want_flags |= TX_CON_CLO_SET;
		}

		if (want_flags != (txn->flags & (TX_CON_CLO_SET|TX_CON_KAL_SET)))
			http_change_connection_header(txn, msg, want_flags);
	}

 skip_header_mangling:
	/* Always enter in the body analyzer */
	rep->analysers &= ~AN_RES_FLT_XFER_DATA;
	rep->analysers |= AN_RES_HTTP_XFER_BODY;

	/* if the user wants to log as soon as possible, without counting
	 * bytes from the server, then this is the right moment. We have
	 * to temporarily assign bytes_out to log what we currently have.
	 */
	if (!LIST_ISEMPTY(&sess->fe->logformat) && !(s->logs.logwait & LW_BYTES)) {
		s->logs.t_close = s->logs.t_data; /* to get a valid end date */
		s->logs.bytes_out = txn->rsp.eoh;
		s->do_log(s);
		s->logs.bytes_out = 0;
	}
	return 1;
}
