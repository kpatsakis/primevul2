uuid_make(char *str)
{
  uuid_t uu;

  uuid_generate_random(uu);
  uuid_unparse_upper(uu, str);
}