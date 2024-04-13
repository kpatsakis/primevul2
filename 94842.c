verify (NMSetting *setting, GSList *all_settings, GError **error)
{
	NMSettingVPNPrivate *priv = NM_SETTING_VPN_GET_PRIVATE (setting);

	if (!priv->service_type) {
		g_set_error (error,
		             NM_SETTING_VPN_ERROR,
		             NM_SETTING_VPN_ERROR_MISSING_PROPERTY,
		             NM_SETTING_VPN_SERVICE_TYPE);
		return FALSE;
	}

	if (!strlen (priv->service_type)) {
		g_set_error (error,
		             NM_SETTING_VPN_ERROR,
		             NM_SETTING_VPN_ERROR_INVALID_PROPERTY,
		             NM_SETTING_VPN_SERVICE_TYPE);
		return FALSE;
	}

	/* default username can be NULL, but can't be zero-length */
	if (priv->user_name && !strlen (priv->user_name)) {
		g_set_error (error,
		             NM_SETTING_VPN_ERROR,
		             NM_SETTING_VPN_ERROR_INVALID_PROPERTY,
		             NM_SETTING_VPN_USER_NAME);
		return FALSE;
	}

	return TRUE;
}
