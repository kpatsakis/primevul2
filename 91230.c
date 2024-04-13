int XfrmController::fillNlAttrXfrmAlgoEnc(const XfrmAlgo& inAlgo, nlattr_algo_crypt* algo) {
 if (inAlgo.name.empty()) { // Do not fill anything if algorithm not provided
 return 0;
 }

 int len = NLA_HDRLEN + sizeof(xfrm_algo);
    strncpy(algo->crypt.alg_name, inAlgo.name.c_str(), sizeof(algo->crypt.alg_name));
    algo->crypt.alg_key_len = inAlgo.key.size() * 8; // bits
    memcpy(algo->key, &inAlgo.key[0], inAlgo.key.size());
    len += inAlgo.key.size();
    fillXfrmNlaHdr(&algo->hdr, XFRMA_ALG_CRYPT, len);
 return len;
}
