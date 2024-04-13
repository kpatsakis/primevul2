static unsigned int fanout_demux_hash(struct packet_fanout *f,
				      struct sk_buff *skb,
				      unsigned int num)
{
	return reciprocal_divide(skb->rxhash, num);
}