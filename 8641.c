Proto_ConcatXMLStrings(gchar *str1,
                       gchar *str2)
{
   gchar *newStr;

   if (NULL == str2) {
      return str1;
   }
   newStr = g_strdup_printf("%s%s", str1, str2);
   g_free(str1);
   g_free(str2);

   return newStr;
}