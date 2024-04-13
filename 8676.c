static void mctp_serial_rx(struct mctp_serial *dev)
{
	struct mctp_skb_cb *cb;
	struct sk_buff *skb;

	if (dev->rxfcs != dev->rxfcs_rcvd) {
		dev->netdev->stats.rx_dropped++;
		dev->netdev->stats.rx_crc_errors++;
		return;
	}

	skb = netdev_alloc_skb(dev->netdev, dev->rxlen);
	if (!skb) {
		dev->netdev->stats.rx_dropped++;
		return;
	}

	skb->protocol = htons(ETH_P_MCTP);
	skb_put_data(skb, dev->rxbuf, dev->rxlen);
	skb_reset_network_header(skb);

	cb = __mctp_cb(skb);
	cb->halen = 0;

	netif_rx_ni(skb);
	dev->netdev->stats.rx_packets++;
	dev->netdev->stats.rx_bytes += dev->rxlen;
}