void Browser::SetAsDelegate(WebContents* web_contents, bool set_delegate) {
  Browser* delegate = set_delegate ? this : nullptr;

  web_contents->SetDelegate(delegate);

  WebContentsModalDialogManager::FromWebContents(web_contents)->
      SetDelegate(delegate);
  CoreTabHelper::FromWebContents(web_contents)->set_delegate(delegate);
  translate::ContentTranslateDriver& content_translate_driver =
      ChromeTranslateClient::FromWebContents(web_contents)->translate_driver();
  if (delegate) {
    zoom::ZoomController::FromWebContents(web_contents)->AddObserver(this);
    content_translate_driver.AddObserver(this);
    BookmarkTabHelper::FromWebContents(web_contents)->AddObserver(this);
  } else {
    zoom::ZoomController::FromWebContents(web_contents)->RemoveObserver(this);
    content_translate_driver.RemoveObserver(this);
    BookmarkTabHelper::FromWebContents(web_contents)->RemoveObserver(this);
  }
}
