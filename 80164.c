void LoginPromptBrowserTestObserver::AddHandler(LoginHandler* handler) {
  std::list<LoginHandler*>::iterator i = std::find(handlers_.begin(),
                                                   handlers_.end(),
                                                   handler);
  EXPECT_TRUE(i == handlers_.end());
  if (i == handlers_.end())
    handlers_.push_back(handler);
}
