set_colour_code(char *str, char **var)
{
    char *keyseq;
    int len;

    zsfree(*var);
    keyseq = getkeystring(str, &len, GETKEYS_BINDKEY, NULL);
    *var = metafy(keyseq, len, META_DUP);
}