cmdpop(void)
{
    if (cmdsp <= 0) {
	DPUTS(1, "BUG: cmdstack empty");
	fflush(stderr);
    } else
	cmdsp--;
}