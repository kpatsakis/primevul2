static void bundle_destroy(struct connectbundle *bundle)
{
  if(!bundle)
    return;

  Curl_llist_destroy(&bundle->conn_list, NULL);

  free(bundle);
}