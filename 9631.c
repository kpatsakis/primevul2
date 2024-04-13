PJ_DEF(pj_status_t) pjstun_create_bind_req( pj_pool_t *pool, 
					     void **msg, pj_size_t *len,
					     pj_uint32_t id_hi, 
					     pj_uint32_t id_lo)
{
    pjstun_msg_hdr *hdr;
    
    PJ_CHECK_STACK();


    hdr = PJ_POOL_ZALLOC_T(pool, pjstun_msg_hdr);
    if (!hdr)
	return PJ_ENOMEM;

    hdr->type = pj_htons(PJSTUN_BINDING_REQUEST);
    hdr->tsx[2] = pj_htonl(id_hi);
    hdr->tsx[3] = pj_htonl(id_lo);
    *msg = hdr;
    *len = sizeof(pjstun_msg_hdr);

    return PJ_SUCCESS;
}