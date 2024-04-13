int _yr_scan_verify_chained_string_match(
    YR_STRING* matching_string,
    YR_SCAN_CONTEXT* context,
    uint8_t* match_data,
    uint64_t match_base,
    uint64_t match_offset,
    int32_t match_length)
{
  YR_STRING* string;
  YR_MATCH* match;
  YR_MATCH* next_match;
  YR_MATCH* new_match;

  uint64_t lower_offset;
  uint64_t ending_offset;
  int32_t full_chain_length;

  int tidx = context->tidx;
  int add_match = FALSE;

  if (matching_string->chained_to == NULL)
  {
    add_match = TRUE;
  }
  else
  {
    if (matching_string->unconfirmed_matches[tidx].head != NULL)
      lower_offset = matching_string->unconfirmed_matches[tidx].head->offset;
    else
      lower_offset = match_offset;

    match = matching_string->chained_to->unconfirmed_matches[tidx].head;

    while (match != NULL)
    {
      next_match = match->next;
      ending_offset = match->offset + match->match_length;

      if (ending_offset + matching_string->chain_gap_max < lower_offset)
      {
        _yr_scan_remove_match_from_list(
            match, &matching_string->chained_to->unconfirmed_matches[tidx]);
      }
      else
      {
        if (ending_offset + matching_string->chain_gap_max >= match_offset &&
            ending_offset + matching_string->chain_gap_min <= match_offset)
        {
          add_match = TRUE;
          break;
        }
      }

      match = next_match;
    }
  }

  if (add_match)
  {
    if (STRING_IS_CHAIN_TAIL(matching_string))
    {
      assert(matching_string->chained_to != NULL);

      match = matching_string->chained_to->unconfirmed_matches[tidx].head;

      while (match != NULL)
      {
        ending_offset = match->offset + match->match_length;

        if (ending_offset + matching_string->chain_gap_max >= match_offset &&
            ending_offset + matching_string->chain_gap_min <= match_offset)
        {
          _yr_scan_update_match_chain_length(
              tidx, matching_string->chained_to, match, 1);
        }

        match = match->next;
      }

      full_chain_length = 0;
      string = matching_string;

      while(string->chained_to != NULL)
      {
        full_chain_length++;
        string = string->chained_to;
      }


      match = string->unconfirmed_matches[tidx].head;

      while (match != NULL)
      {
        next_match = match->next;

        if (match->chain_length == full_chain_length)
        {
          _yr_scan_remove_match_from_list(
              match, &string->unconfirmed_matches[tidx]);

          match->match_length = (int32_t) \
              (match_offset - match->offset + match_length);

          match->data_length = yr_min(match->match_length, MAX_MATCH_DATA);

          FAIL_ON_ERROR(yr_arena_write_data(
              context->matches_arena,
              match_data - match_offset + match->offset,
              match->data_length,
              (void**) &match->data));

          FAIL_ON_ERROR(_yr_scan_add_match_to_list(
              match, &string->matches[tidx], FALSE));
        }

        match = next_match;
      }
    }
    else
    {
      if (matching_string->matches[tidx].count == 0 &&
          matching_string->unconfirmed_matches[tidx].count == 0)
      {

        FAIL_ON_ERROR(yr_arena_write_data(
            context->matching_strings_arena,
            &matching_string,
            sizeof(matching_string),
            NULL));
      }

      FAIL_ON_ERROR(yr_arena_allocate_memory(
          context->matches_arena,
          sizeof(YR_MATCH),
          (void**) &new_match));

      new_match->data_length = yr_min(match_length, MAX_MATCH_DATA);

      FAIL_ON_ERROR(yr_arena_write_data(
          context->matches_arena,
          match_data,
          new_match->data_length,
          (void**) &new_match->data));

      new_match->base = match_base;
      new_match->offset = match_offset;
      new_match->match_length = match_length;
      new_match->chain_length = 0;
      new_match->prev = NULL;
      new_match->next = NULL;

      FAIL_ON_ERROR(_yr_scan_add_match_to_list(
          new_match,
          &matching_string->unconfirmed_matches[tidx],
          FALSE));
    }
  }

  return ERROR_SUCCESS;
}
