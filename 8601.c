static void bundle_add_conn(struct connectbundle *bundle,
                            struct connectdata *conn)
{
  Curl_llist_insert_next(&bundle->conn_list, bundle->conn_list.tail, conn,
                         &conn->bundle_node);
  conn->bundle = bundle;
  bundle->num_connections++;
}