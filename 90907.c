static int override_release(char __user *release, size_t len)
{
 int ret = 0;

 if (current->personality & UNAME26) {
 const char *rest = UTS_RELEASE;
 char buf[65] = { 0 };
 int ndots = 0;
 unsigned v;
 size_t copy;

 while (*rest) {
 if (*rest == '.' && ++ndots >= 3)
 break;
 if (!isdigit(*rest) && *rest != '.')
 break;
			rest++;
 }
		v = ((LINUX_VERSION_CODE >> 8) & 0xff) + 40;
		copy = clamp_t(size_t, len, 1, sizeof(buf));
		copy = scnprintf(buf, copy, "2.6.%u%s", v, rest);
		ret = copy_to_user(release, buf, copy + 1);
 }
 return ret;
}
