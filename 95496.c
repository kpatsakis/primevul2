i915_gem_execbuffer_relocate_object(struct drm_i915_gem_object *obj,
				    struct eb_objects *eb)
{
	struct drm_i915_gem_relocation_entry __user *user_relocs;
	struct drm_i915_gem_exec_object2 *entry = obj->exec_entry;
	int i, ret;

	user_relocs = (void __user *)(uintptr_t)entry->relocs_ptr;
	for (i = 0; i < entry->relocation_count; i++) {
		struct drm_i915_gem_relocation_entry reloc;

		if (__copy_from_user_inatomic(&reloc,
					      user_relocs+i,
					      sizeof(reloc)))
			return -EFAULT;

		ret = i915_gem_execbuffer_relocate_entry(obj, eb, &reloc);
		if (ret)
			return ret;

		if (__copy_to_user_inatomic(&user_relocs[i].presumed_offset,
					    &reloc.presumed_offset,
					    sizeof(reloc.presumed_offset)))
			return -EFAULT;
	}

	return 0;
}
