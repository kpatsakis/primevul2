 addrs_in_same_network_family(const tor_addr_t *a1,
                              const tor_addr_t *a2)
 {
  return 0 == tor_addr_compare_masked(a1, a2, 16, CMP_SEMANTIC);
}
