net_connect(const char *addr, unsigned short port, int type, const char *log_service_name)
{
  struct addrinfo hints = { 0 };
  struct addrinfo *servinfo;
  struct addrinfo *ptr;
  char strport[8];
  int fd;
  int ret;

  DPRINTF(E_DBG, L_MISC, "Connecting to '%s' at %s (port %u)\n", log_service_name, addr, port);

  hints.ai_socktype = (type & (SOCK_STREAM | SOCK_DGRAM)); // filter since type can be SOCK_STREAM | SOCK_NONBLOCK
  hints.ai_family = (cfg_getbool(cfg_getsec(cfg, "general"), "ipv6")) ? AF_UNSPEC : AF_INET;

  snprintf(strport, sizeof(strport), "%hu", port);
  ret = getaddrinfo(addr, strport, &hints, &servinfo);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_MISC, "Could not connect to '%s' at %s (port %u): %s\n", log_service_name, addr, port, gai_strerror(ret));
      return -1;
    }

  for (ptr = servinfo; ptr; ptr = ptr->ai_next)
    {
      fd = socket(ptr->ai_family, type | SOCK_CLOEXEC, ptr->ai_protocol);
      if (fd < 0)
	{
	  continue;
	}

      ret = connect(fd, ptr->ai_addr, ptr->ai_addrlen);
      if (ret < 0 && errno != EINPROGRESS) // EINPROGRESS in case of SOCK_NONBLOCK
	{
	  close(fd);
	  continue;
	}

      break;
    }

  freeaddrinfo(servinfo);

  if (!ptr)
    {
      DPRINTF(E_LOG, L_MISC, "Could not connect to '%s' at %s (port %u): %s\n", log_service_name, addr, port, strerror(errno));
      return -1;
    }

  // net_address_get(ipaddr, sizeof(ipaddr), (union net_sockaddr *)ptr->ai-addr);

  return fd;
}