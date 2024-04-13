static int crypto_report_kpp(struct sk_buff *skb, struct crypto_alg *alg)
 {
 	struct crypto_report_kpp rkpp;
 
	strncpy(rkpp.type, "kpp", sizeof(rkpp.type));
 
 	if (nla_put(skb, CRYPTOCFGA_REPORT_KPP,
 		    sizeof(struct crypto_report_kpp), &rkpp))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -EMSGSIZE;
}
