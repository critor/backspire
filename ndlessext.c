#include <os.h>
#include "nand.h"

// indexes of OSes as used in Ndless
enum {
  OSCL31N,OSCL31C,OSCX31N,OSCX31C,OSCM31N,OSCM31C,
  OSCL36N,OSCL36C,OSCX36N,OSCX36C,
  OSCL390N,OSCL390C,OSCX390N,OSCX390C,
  OSCL391N,OSCL391C,OSCX391N,OSCX391C,
  OSCX400N,OSCX400C,
  OSCX403N,OSCX403C,
  OSCX42N,OSCX42C,
  OSCX43N,OSCX43C,
  OSCX44N,OSCX44C,
  OSCX450N,OSCX450C,
  OSCX451N,OSCX451C,
  OSCX453N,OSCX453C,
  OSCXII52N,OSCXII52T,OSCXII52C,
  OSCX454N,OSCX454C,
  OSCXII53N,OSCXII53T,OSCXII53C,
};
#define NOS (OSCXII53C+1)

// get current OS index for Ndless
int get_os() {
  int vals[NOS];
  for(int i=0; i<NOS; i++)
    vals[i] = i;
  return nl_osvalue(vals, NOS);
}

// force custom adresses for some CX2 syscalls (3 adresses for OSes 5.2 + 3 adresses for OSes 5.3)
static const unsigned read_nand_addrs[]  = {0x10071F5C, 0x10071EC4, 0x10071658, 0X100715E8, 0x1006E0AC, 0x1006E03C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X1001B67C, 0X1001B67C, 0X1001B67C, 0, 0, 0X1001B6BC, 0X1001B6BC, 0X1001B6BC};
static const unsigned write_nand_addrs[] = {0x10072298, 0x10072200, 0x10071994, 0x10071924, 0x1006E3E8, 0x1006E378, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X1001BE74, 0X1001BE74, 0X1001BE74, 0, 0, 0X1001BEB4, 0X1001BEB4, 0X1001BEB4};
static const unsigned erase_nand_addrs[] = {0x100724FC, 0x10072464, 0x10071BF8, 0x10071B88, 0x1006E64C, 0x1006E5DC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X1001B5B8, 0X1001B5B8, 0X1001B5B8, 0, 0, 0X1001B5F8, 0X1001B5F8, 0X1001B5F8};

// matching custom syscalls
#define read_nand_31		SYSCALL_CUSTOM(read_nand_addrs,void,  void* dest, int size, int offset, int, int percent_max, void *progress_cb)
#define write_nand_31		SYSCALL_CUSTOM(write_nand_addrs,int,  void *source,int size, unsigned int offset)
#define write_nand_cx2		SYSCALL_CUSTOM(write_nand_addrs,int,  unsigned int offset, void *source,int size)
#define erase_nand_31		SYSCALL_CUSTOM(erase_nand_addrs,int, int offset,int end)
#define erase_nand_cx2		SYSCALL_CUSTOM(erase_nand_addrs,int, int offset,int blocks)

// call Ndless builtin syscall or our custom syscall, depending on the running OS index
void ext_read_nand(void* dest, int size, int offset, int unknown, int percent_max, void *progress_cb) {
  int os = get_os();
  if (os >= OSCL31N && os <=OSCM31C && nl_ndless_rev() < 989) // Ndless 3.1
    read_nand_31(dest, size, offset, unknown, percent_max, progress_cb);
  else
    read_nand(dest, size, offset, unknown, percent_max, progress_cb);
}

int ext_write_nand(void *source,int size, unsigned int offset) {
  int os = get_os();
  if((os >= OSCXII52N && os <= OSCXII52C) || (os >= OSCXII53N && os <= OSCXII53C))
    write_nand_cx2(offset, source, size);
  else if (os >= OSCL31N && os <=OSCM31C && nl_ndless_rev() < 989) // Ndless 3.1
    write_nand_31(source, size, offset);
  else
    write_nand(source, size, offset);
}

int ext_erase_nand(int offset,int end) {
  int os = get_os();
  if((os >= OSCXII52N && os <= OSCXII52C) || (os >= OSCXII53N && os <= OSCXII53C)) {
    int blocks = (int)((end+1-offset)/NAND_PAGE_SIZE/0x40);
    erase_nand_cx2(offset, blocks);
  }
  else if (os >= OSCL31N && os <=OSCM31C && nl_ndless_rev() < 989) // Ndless 3.1
    erase_nand_31(offset, end);
  else
    nand_erase_range(offset, end);
}

