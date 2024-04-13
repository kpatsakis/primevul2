void TabSpecificContentSettings::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK(type == chrome::NOTIFICATION_CONTENT_SETTINGS_CHANGED);

  content::Details<const ContentSettingsDetails> settings_details(details);
  const NavigationController& controller = web_contents()->GetController();
  NavigationEntry* entry = controller.GetActiveEntry();
  GURL entry_url;
  if (entry)
    entry_url = entry->GetURL();
  if (settings_details.ptr()->update_all() ||
      settings_details.ptr()->primary_pattern().Matches(entry_url)) {
    Profile* profile =
        Profile::FromBrowserContext(web_contents()->GetBrowserContext());
    RendererContentSettingRules rules;
    GetRendererContentSettingRules(profile->GetHostContentSettingsMap(),
                                   &rules);
    Send(new ChromeViewMsg_SetContentSettingRules(rules));
  }
}
