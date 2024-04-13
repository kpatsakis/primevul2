Proto_RequestNameToType(const gchar *name)
{
   int i;

   for (i = 0; i < G_N_ELEMENTS(reqNameList); i++) {
      if (g_strcmp0(name, reqNameList[i].reqName) == 0) {
         return reqNameList[i].type;
      }
   }

   return PROTO_REQUEST_UNKNOWN;
}