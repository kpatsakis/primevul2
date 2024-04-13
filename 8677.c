static netdev_tx_t mctp_serial_tx(struct sk_buff *skb, struct net_device *ndev)
{
	struct mctp_serial *dev = netdev_priv(ndev);
	unsigned long flags;

	WARN_ON(dev->txstate != STATE_IDLE);

	if (skb->len > MCTP_SERIAL_MTU) {
		dev->netdev->stats.tx_dropped++;
		goto out;
	}

	spin_lock_irqsave(&dev->lock, flags);
	netif_stop_queue(dev->netdev);
	skb_copy_bits(skb, 0, dev->txbuf, skb->len);
	dev->txpos = 0;
	dev->txlen = skb->len;
	dev->txstate = STATE_START;
	spin_unlock_irqrestore(&dev->lock, flags);

	set_bit(TTY_DO_WRITE_WAKEUP, &dev->tty->flags);
	schedule_work(&dev->tx_work);

out:
	kfree_skb(skb);
	return NETDEV_TX_OK;
}