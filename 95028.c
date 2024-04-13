int http_wait_for_request_body(struct stream *s, struct channel *req, int an_bit)
{
	struct session *sess = s->sess;
	struct http_txn *txn = s->txn;
	struct http_msg *msg = &s->txn->req;

	/* We have to parse the HTTP request body to find any required data.
	 * "balance url_param check_post" should have been the only way to get
	 * into this. We were brought here after HTTP header analysis, so all
	 * related structures are ready.
	 */

	if (msg->msg_state < HTTP_MSG_CHUNK_SIZE) {
		/* This is the first call */
		if (msg->msg_state < HTTP_MSG_BODY)
			goto missing_data;

		if (msg->msg_state < HTTP_MSG_100_SENT) {
			/* If we have HTTP/1.1 and Expect: 100-continue, then we must
			 * send an HTTP/1.1 100 Continue intermediate response.
			 */
			if (msg->flags & HTTP_MSGF_VER_11) {
				struct hdr_ctx ctx;
				ctx.idx = 0;
				/* Expect is allowed in 1.1, look for it */
				if (http_find_header2("Expect", 6, req->buf->p, &txn->hdr_idx, &ctx) &&
				    unlikely(ctx.vlen == 12 && strncasecmp(ctx.line+ctx.val, "100-continue", 12) == 0)) {
					co_inject(&s->res, http_100_chunk.str, http_100_chunk.len);
					http_remove_header2(&txn->req, &txn->hdr_idx, &ctx);
				}
			}
			msg->msg_state = HTTP_MSG_100_SENT;
		}

		/* we have msg->sov which points to the first byte of message body.
		 * req->buf->p still points to the beginning of the message. We
		 * must save the body in msg->next because it survives buffer
		 * re-alignments.
		 */
		msg->next = msg->sov;

		if (msg->flags & HTTP_MSGF_TE_CHNK)
			msg->msg_state = HTTP_MSG_CHUNK_SIZE;
		else
			msg->msg_state = HTTP_MSG_DATA;
	}

	if (!(msg->flags & HTTP_MSGF_TE_CHNK)) {
		/* We're in content-length mode, we just have to wait for enough data. */
		if (http_body_bytes(msg) < msg->body_len)
			goto missing_data;

		/* OK we have everything we need now */
		goto http_end;
	}

	/* OK here we're parsing a chunked-encoded message */

	if (msg->msg_state == HTTP_MSG_CHUNK_SIZE) {
		/* read the chunk size and assign it to ->chunk_len, then
		 * set ->sov and ->next to point to the body and switch to DATA or
		 * TRAILERS state.
		 */
		unsigned int chunk;
		int ret = h1_parse_chunk_size(req->buf, msg->next, req->buf->i, &chunk);

		if (!ret)
			goto missing_data;
		else if (ret < 0) {
			msg->err_pos = req->buf->i + ret;
			if (msg->err_pos < 0)
				msg->err_pos += req->buf->size;
			stream_inc_http_err_ctr(s);
			goto return_bad_req;
		}

		msg->chunk_len = chunk;
		msg->body_len += chunk;

		msg->sol = ret;
		msg->next += ret;
		msg->msg_state = msg->chunk_len ? HTTP_MSG_DATA : HTTP_MSG_TRAILERS;
	}

	/* Now we're in HTTP_MSG_DATA or HTTP_MSG_TRAILERS state.
	 * We have the first data byte is in msg->sov + msg->sol. We're waiting
	 * for at least a whole chunk or the whole content length bytes after
	 * msg->sov + msg->sol.
	 */
	if (msg->msg_state == HTTP_MSG_TRAILERS)
		goto http_end;

	if (http_body_bytes(msg) >= msg->body_len)   /* we have enough bytes now */
		goto http_end;

 missing_data:
	/* we get here if we need to wait for more data. If the buffer is full,
	 * we have the maximum we can expect.
	 */
	if (buffer_full(req->buf, global.tune.maxrewrite))
		goto http_end;

	if ((req->flags & CF_READ_TIMEOUT) || tick_is_expired(req->analyse_exp, now_ms)) {
		txn->status = 408;
		http_reply_and_close(s, txn->status, http_error_message(s));

		if (!(s->flags & SF_ERR_MASK))
			s->flags |= SF_ERR_CLITO;
		if (!(s->flags & SF_FINST_MASK))
			s->flags |= SF_FINST_D;
		goto return_err_msg;
	}

	/* we get here if we need to wait for more data */
	if (!(req->flags & (CF_SHUTR | CF_READ_ERROR))) {
		/* Not enough data. We'll re-use the http-request
		 * timeout here. Ideally, we should set the timeout
		 * relative to the accept() date. We just set the
		 * request timeout once at the beginning of the
		 * request.
		 */
		channel_dont_connect(req);
		if (!tick_isset(req->analyse_exp))
			req->analyse_exp = tick_add_ifset(now_ms, s->be->timeout.httpreq);
		return 0;
	}

 http_end:
	/* The situation will not evolve, so let's give up on the analysis. */
	s->logs.tv_request = now;  /* update the request timer to reflect full request */
	req->analysers &= ~an_bit;
	req->analyse_exp = TICK_ETERNITY;
	return 1;

 return_bad_req: /* let's centralize all bad requests */
	txn->req.err_state = txn->req.msg_state;
	txn->req.msg_state = HTTP_MSG_ERROR;
	txn->status = 400;
	http_reply_and_close(s, txn->status, http_error_message(s));

	if (!(s->flags & SF_ERR_MASK))
		s->flags |= SF_ERR_PRXCOND;
	if (!(s->flags & SF_FINST_MASK))
		s->flags |= SF_FINST_R;

 return_err_msg:
	req->analysers &= AN_REQ_FLT_END;
	HA_ATOMIC_ADD(&sess->fe->fe_counters.failed_req, 1);
	if (sess->listener->counters)
		HA_ATOMIC_ADD(&sess->listener->counters->failed_req, 1);
	return 0;
}
