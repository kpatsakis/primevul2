m_readfile(const char *path, int num_lines)
{
  char buf[256];
  FILE *fp;
  char **lines;
  char *line;
  int i;

  // Alloc array of char pointers
  lines = calloc(num_lines, sizeof(char *));
  if (!lines)
    return NULL;

  fp = fopen(path, "rb");
  if (!fp)
    {
      DPRINTF(E_LOG, L_MISC, "Could not open file '%s' for reading: %s\n", path, strerror(errno));
      free(lines);
      return NULL;
    }

  for (i = 0; i < num_lines; i++)
    {
      line = fgets(buf, sizeof(buf), fp);
      if (!line)
	{
	  DPRINTF(E_LOG, L_MISC, "File '%s' has fewer lines than expected (found %d, expected %d)\n", path, i, num_lines);
	  goto error;
	}

      lines[i] = atrim(line);
      if (!lines[i] || (strlen(lines[i]) == 0))
	{
	  DPRINTF(E_LOG, L_MISC, "Line %d in '%s' is invalid\n", i+1, path);
	  goto error;
	}
    }

  fclose(fp);

  return lines;

 error:
  for (i = 0; i < num_lines; i++)
    free(lines[i]);

  free(lines);
  fclose(fp);
  return NULL;
}