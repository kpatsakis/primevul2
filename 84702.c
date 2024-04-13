void Browser::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  switch (type) {
    case extensions::NOTIFICATION_EXTENSION_PROCESS_TERMINATED: {
      Profile* profile = content::Source<Profile>(source).ptr();
      if (profile_->IsSameProfile(profile) && window()->GetLocationBar())
        window()->GetLocationBar()->UpdatePageActions();
      break;
    }

#if !defined(OS_ANDROID)
    case chrome::NOTIFICATION_BROWSER_THEME_CHANGED:
      window()->UserChangedTheme();
      break;
#endif

    case chrome::NOTIFICATION_WEB_CONTENT_SETTINGS_CHANGED: {
      WebContents* web_contents = content::Source<WebContents>(source).ptr();
      if (web_contents == tab_strip_model_->GetActiveWebContents()) {
        LocationBar* location_bar = window()->GetLocationBar();
        if (location_bar)
          location_bar->UpdateContentSettingsIcons();
      }
      break;
    }

    default:
      NOTREACHED() << "Got a notification we didn't register for.";
  }
}
