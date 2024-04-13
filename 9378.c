keyval_sort(struct keyval *kv)
{
  struct onekeyval *head;
  struct onekeyval *okv;
  struct onekeyval *sokv;

  if (!kv || !kv->head)
    return;

  head = kv->head;
  for (okv = kv->head; okv; okv = okv->next)
    {
      okv->sort = NULL;
      for (sokv = kv->head; sokv; sokv = sokv->next)
	{
	  // We try to find a name which is greater than okv->name
	  // but less than our current candidate (okv->sort->name)
	  if ( (strcmp(sokv->name, okv->name) > 0) &&
	       ((okv->sort == NULL) || (strcmp(sokv->name, okv->sort->name) < 0)) )
	    okv->sort = sokv;
	}

      // Find smallest name, which will be the new head
      if (strcmp(okv->name, head->name) < 0)
	head = okv;
    }

  while ((okv = kv->head))
    {
      kv->head  = okv->next;
      okv->next = okv->sort;
    }

  kv->head = head;
  for (okv = kv->head; okv; okv = okv->next)
    kv->tail = okv;

  DPRINTF(E_DBG, L_MISC, "Keyval sorted. New head: %s. New tail: %s.\n", kv->head->name, kv->tail->name);
}