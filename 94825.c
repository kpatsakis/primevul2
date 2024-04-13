copy_attr (char const *src_path, char const *dst_path)
{
  struct error_context ctx =
  {
    .error = copy_attr_error,
    .quote = copy_attr_quote,
    .quote_free = copy_attr_free
  };
  /* FIXME: We are copying between files we know we can safely access by
   * pathname. A safe_ version of attr_copy_file() might still be slightly
   * more efficient for deep paths. */
  return attr_copy_file (src_path, dst_path, copy_attr_check, &ctx);
}
