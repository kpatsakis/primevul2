net_address_get(char *addr, size_t addr_len, union net_sockaddr *naddr)
{
  const char *s;

  memset(addr, 0, addr_len); // Just in case caller doesn't check for errors

  if (naddr->sa.sa_family == AF_INET6)
     s = inet_ntop(AF_INET6, &naddr->sin6.sin6_addr, addr, addr_len);
  else
     s = inet_ntop(AF_INET, &naddr->sin.sin_addr, addr, addr_len);

  if (!s)
    return -1;

  return 0;
}