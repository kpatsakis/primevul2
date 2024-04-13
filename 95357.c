int __glXDisp_RenderLarge(__GLXclientState *cl, GLbyte *pc)
{
    xGLXRenderLargeReq *req;
    ClientPtr client= cl->client;
    size_t dataBytes;
    __GLXrenderLargeHeader *hdr;
    __GLXcontext *glxc;
    int error;
    CARD16 opcode;
    __GLX_DECLARE_SWAP_VARIABLES;
    
    req = (xGLXRenderLargeReq *) pc;
    if (client->swapped) {
	__GLX_SWAP_SHORT(&req->length);
	__GLX_SWAP_INT(&req->contextTag);
	__GLX_SWAP_INT(&req->dataBytes);
	__GLX_SWAP_SHORT(&req->requestNumber);
	__GLX_SWAP_SHORT(&req->requestTotal);
    }

    glxc = __glXForceCurrent(cl, req->contextTag, &error);
    if (!glxc) {
	/* Reset in case this isn't 1st request. */
	__glXResetLargeCommandStatus(cl);
	return error;
    }
    dataBytes = req->dataBytes;

    /*
    ** Check the request length.
    */
    if ((req->length << 2) != __GLX_PAD(dataBytes) + sz_xGLXRenderLargeReq) {
	client->errorValue = req->length;
	/* Reset in case this isn't 1st request. */
	__glXResetLargeCommandStatus(cl);
	return BadLength;
    }
    pc += sz_xGLXRenderLargeReq;
    
    if (cl->largeCmdRequestsSoFar == 0) {
	__GLXrenderSizeData entry;
	int extra;
	size_t cmdlen;
	int err;

	/*
	** This is the first request of a multi request command.
	** Make enough space in the buffer, then copy the entire request.
	*/
	if (req->requestNumber != 1) {
	    client->errorValue = req->requestNumber;
	    return __glXError(GLXBadLargeRequest);
	}

	hdr = (__GLXrenderLargeHeader *) pc;
	if (client->swapped) {
	    __GLX_SWAP_INT(&hdr->length);
	    __GLX_SWAP_INT(&hdr->opcode);
	}
	cmdlen = hdr->length;
	opcode = hdr->opcode;

	/*
	** Check for core opcodes and grab entry data.
	*/
	err = __glXGetProtocolSizeData(& Render_dispatch_info, opcode, & entry);
	if (err < 0) {
	    client->errorValue = opcode;
	    return __glXError(GLXBadLargeRequest);
	}

	if (entry.varsize) {
	    /*
	    ** If it's a variable-size command (a command whose length must
	    ** be computed from its parameters), all the parameters needed
	    ** will be in the 1st request, so it's okay to do this.
	    */
	    extra = (*entry.varsize)(pc + __GLX_RENDER_LARGE_HDR_SIZE,
				     client->swapped);
	    if (extra < 0) {
		extra = 0;
	    }
	    /* large command's header is 4 bytes longer, so add 4 */
	    if (cmdlen != __GLX_PAD(entry.bytes + 4 + extra)) {
		return BadLength;
	    }
	} else {
	    /* constant size command */
	    if (cmdlen != __GLX_PAD(entry.bytes + 4)) {
		return BadLength;
	    }
	}
	/*
	** Make enough space in the buffer, then copy the entire request.
	*/
	if (cl->largeCmdBufSize < cmdlen) {
	    if (!cl->largeCmdBuf) {
		cl->largeCmdBuf = (GLbyte *) malloc(cmdlen);
	    } else {
		cl->largeCmdBuf = (GLbyte *) realloc(cl->largeCmdBuf, cmdlen);
	    }
	    if (!cl->largeCmdBuf) {
		return BadAlloc;
	    }
	    cl->largeCmdBufSize = cmdlen;
	}
	memcpy(cl->largeCmdBuf, pc, dataBytes);

	cl->largeCmdBytesSoFar = dataBytes;
	cl->largeCmdBytesTotal = cmdlen;
	cl->largeCmdRequestsSoFar = 1;
	cl->largeCmdRequestsTotal = req->requestTotal;
	return Success;
	
    } else {
	/*
	** We are receiving subsequent (i.e. not the first) requests of a
	** multi request command.
	*/

	/*
	** Check the request number and the total request count.
	*/
	if (req->requestNumber != cl->largeCmdRequestsSoFar + 1) {
	    client->errorValue = req->requestNumber;
	    __glXResetLargeCommandStatus(cl);
	    return __glXError(GLXBadLargeRequest);
	}
	if (req->requestTotal != cl->largeCmdRequestsTotal) {
	    client->errorValue = req->requestTotal;
	    __glXResetLargeCommandStatus(cl);
	    return __glXError(GLXBadLargeRequest);
	}

	/*
	** Check that we didn't get too much data.
	*/
	if ((cl->largeCmdBytesSoFar + dataBytes) > cl->largeCmdBytesTotal) {
	    client->errorValue = dataBytes;
	    __glXResetLargeCommandStatus(cl);
	    return __glXError(GLXBadLargeRequest);
	}
	memcpy(cl->largeCmdBuf + cl->largeCmdBytesSoFar, pc, dataBytes);
	cl->largeCmdBytesSoFar += dataBytes;
	cl->largeCmdRequestsSoFar++;

	if (req->requestNumber == cl->largeCmdRequestsTotal) {
	    __GLXdispatchRenderProcPtr proc;

	    /*
	    ** This is the last request; it must have enough bytes to complete
	    ** the command.
	    */
	    /* NOTE: the two pad macros have been added below; they are needed
	    ** because the client library pads the total byte count, but not
	    ** the per-request byte counts.  The Protocol Encoding says the
	    ** total byte count should not be padded, so a proposal will be 
	    ** made to the ARB to relax the padding constraint on the total 
	    ** byte count, thus preserving backward compatibility.  Meanwhile, 
	    ** the padding done below fixes a bug that did not allow
	    ** large commands of odd sizes to be accepted by the server.
	    */
	    if (__GLX_PAD(cl->largeCmdBytesSoFar) !=
		__GLX_PAD(cl->largeCmdBytesTotal)) {
		client->errorValue = dataBytes;
		__glXResetLargeCommandStatus(cl);
		return __glXError(GLXBadLargeRequest);
	    }
	    hdr = (__GLXrenderLargeHeader *) cl->largeCmdBuf;
	    /*
	    ** The opcode and length field in the header had already been
	    ** swapped when the first request was received.
	    **
	    ** Use the opcode to index into the procedure table.
	    */
	    opcode = hdr->opcode;

	    proc = (__GLXdispatchRenderProcPtr)
	      __glXGetProtocolDecodeFunction(& Render_dispatch_info, opcode,
					     client->swapped);
	    if (proc == NULL) {
		client->errorValue = opcode;
		return __glXError(GLXBadLargeRequest);
	    }

	    /*
	    ** Skip over the header and execute the command.
	    */
	    (*proc)(cl->largeCmdBuf + __GLX_RENDER_LARGE_HDR_SIZE);
	    __GLX_NOTE_UNFLUSHED_CMDS(glxc);

	    /*
	    ** Reset for the next RenderLarge series.
	    */
	    __glXResetLargeCommandStatus(cl);
	} else {
	    /*
	    ** This is neither the first nor the last request.
	    */
	}
	return Success;
    }
}
