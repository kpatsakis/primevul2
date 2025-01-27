static void avi_metadata_creation_time(AVDictionary **metadata, char *date)
{
    char month[4], time[9], buffer[64];
    int i, day, year;
    /* parse standard AVI date format (ie. "Mon Mar 10 15:04:43 2003") */
    if (sscanf(date, "%*3s%*[ ]%3s%*[ ]%2d%*[ ]%8s%*[ ]%4d",
               month, &day, time, &year) == 4) {
        for (i = 0; i < 12; i++)
            if (!av_strcasecmp(month, months[i])) {
                snprintf(buffer, sizeof(buffer), "%.4d-%.2d-%.2d %s",
                         year, i + 1, day, time);
                av_dict_set(metadata, "creation_time", buffer, 0);
            }
    } else if (date[4] == '/' && date[7] == '/') {
        date[4] = date[7] = '-';
        av_dict_set(metadata, "creation_time", date, 0);
    }
}
