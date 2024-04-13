 onig_new_deluxe(regex_t** reg, const UChar* pattern, const UChar* pattern_end,
                OnigCompileInfo* ci, OnigErrorInfo* einfo)
{
  int r;
  UChar *cpat, *cpat_end;

   if (IS_NOT_NULL(einfo)) einfo->par = (UChar* )NULL;
 
   if (ci->pattern_enc != ci->target_enc) {
    return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
   }
   else {
     cpat     = (UChar* )pattern;
    cpat_end = (UChar* )pattern_end;
  }

  *reg = (regex_t* )xmalloc(sizeof(regex_t));
  if (IS_NULL(*reg)) {
    r = ONIGERR_MEMORY;
    goto err2;
  }

  r = onig_reg_init(*reg, ci->option, ci->case_fold_flag, ci->target_enc,
                    ci->syntax);
  if (r != 0) goto err;

  r = onig_compile(*reg, cpat, cpat_end, einfo);
  if (r != 0) {
  err:
    onig_free(*reg);
    *reg = NULL;
  }

 err2:
  if (cpat != pattern) xfree(cpat);

  return r;
}