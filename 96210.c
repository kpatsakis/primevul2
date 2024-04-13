static void tg3_phy_stop(struct tg3 *tp)
{
	if (!(tp->phy_flags & TG3_PHYFLG_IS_CONNECTED))
		return;

	phy_stop(tp->mdio_bus->phy_map[TG3_PHY_MII_ADDR]);
}
