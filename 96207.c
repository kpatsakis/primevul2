static void tg3_phy_fet_toggle_apd(struct tg3 *tp, bool enable)
{
	u32 phytest;

	if (!tg3_readphy(tp, MII_TG3_FET_TEST, &phytest)) {
		u32 phy;

		tg3_writephy(tp, MII_TG3_FET_TEST,
			     phytest | MII_TG3_FET_SHADOW_EN);
		if (!tg3_readphy(tp, MII_TG3_FET_SHDW_AUXSTAT2, &phy)) {
			if (enable)
				phy |= MII_TG3_FET_SHDW_AUXSTAT2_APD;
			else
				phy &= ~MII_TG3_FET_SHDW_AUXSTAT2_APD;
			tg3_writephy(tp, MII_TG3_FET_SHDW_AUXSTAT2, phy);
		}
		tg3_writephy(tp, MII_TG3_FET_TEST, phytest);
	}
}
