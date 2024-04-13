static bool urlchar_needs_escaping(int c)
{
  return !(ISCNTRL(c) || ISSPACE(c) || ISGRAPH(c));
}