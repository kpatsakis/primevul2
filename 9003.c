cmdpush(int cmdtok)
{
    if (cmdsp >= 0 && cmdsp < CMDSTACKSZ)
	cmdstack[cmdsp++] = (unsigned char)cmdtok;
}