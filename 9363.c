net_evhttp_bind(struct evhttp *evhttp, short unsigned port, const char *log_service_name)
{
  const char *bind_address;
  bool v6_enabled;
  int ret;

  bind_address = cfg_getstr(cfg_getsec(cfg, "general"), "bind_address");
  if (bind_address)
    evhttp_bind_socket(evhttp, bind_address, port);

  // For Linux, we could just do evhttp_bind_socket() for "::", and both the
  // ipv4 and v6 port would be bound. However, for bsd it seems it is necessary
  // to do like below.
  v6_enabled = cfg_getbool(cfg_getsec(cfg, "general"), "ipv6");
  if (v6_enabled)
    {
      ret = evhttp_bind_socket(evhttp, "::", port);
      if (ret < 0)
	{
	  DPRINTF(E_LOG, L_MISC, "Could not bind service '%s' to port %d with IPv6, falling back to IPv4\n", log_service_name, port);
	  v6_enabled = 0;
	}
    }

  ret = evhttp_bind_socket(evhttp, "0.0.0.0", port);
  if (ret < 0)
    {
      if (!v6_enabled)
	return -1;

#ifndef __linux__
      DPRINTF(E_LOG, L_MISC, "Could not bind service '%s' to port %d with IPv4, listening on IPv6 only\n", log_service_name, port);
#endif
    }

  return 0;
}