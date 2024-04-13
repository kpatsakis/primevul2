MagickExport ModuleInfo *AcquireModuleInfo(const char *path,const char *tag)
{
  ModuleInfo
    *module_info;

  module_info=(ModuleInfo *) AcquireMagickMemory(sizeof(*module_info));
  if (module_info == (ModuleInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(module_info,0,sizeof(*module_info));
  if (path != (const char *) NULL)
    module_info->path=ConstantString(path);
  if (tag != (const char *) NULL)
    module_info->tag=ConstantString(tag);
  module_info->timestamp=time(0);
  module_info->signature=MagickSignature;
  return(module_info);
}
