FontFileSortDir(FontDirectoryPtr dir)
{
    FontFileSortTable (&dir->scalable);
    FontFileSortTable (&dir->nonScalable);
    /* now that the table is fixed in size, swizzle the pointers */
    FontFileSwitchStringsToBitmapPointers (dir);
}
