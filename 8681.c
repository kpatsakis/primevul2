static void mctp_serial_uninit(struct net_device *ndev)
{
	struct mctp_serial *dev = netdev_priv(ndev);

	cancel_work_sync(&dev->tx_work);
}