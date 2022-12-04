int get_os();
void ext_read_nand(void* dest, int size, int offset, int unknown, int percent_max, void *progress_cb);
int ext_write_nand(void *source,int size, unsigned int offset);
