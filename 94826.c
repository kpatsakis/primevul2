_dbus_header_get_field_basic (DBusHeader    *header,
                              int            field,
                              int            type,
                              void          *value)
{
  _dbus_assert (field != DBUS_HEADER_FIELD_INVALID);
  _dbus_assert (field <= DBUS_HEADER_FIELD_LAST);
  _dbus_assert (_dbus_header_field_types[field].code == field);
  /* in light of this you might ask why the type is passed in;
   * the only rationale I can think of is so the caller has
   * to specify its expectation and breaks if we change it
   */
  _dbus_assert (type == EXPECTED_TYPE_OF_FIELD (field));

  if (!_dbus_header_cache_check (header, field))
    return FALSE;

  _dbus_assert (header->fields[field].value_pos >= 0);

  _dbus_marshal_read_basic (&header->data,
                            header->fields[field].value_pos,
                            type, value, header->byte_order,
                            NULL);

  return TRUE;
}
