#define NAND_PAGE_SIZE		(has_colors?0x800:0x200)
#define NAND_BLOCK_SIZE		((has_colors?0x40:0x20)*NAND_PAGE_SIZE)
#define NAND_SIZE		((has_colors?128:32)*1024*1024)

#define CLASSIC_CX_NPARTS	5
#define CX2_NPARTS		14

// offsets for the older Nspire CX manuf partition
#define MANUF_PARTTABLE_OFFSET	0x818
#define MANUF_PARTTABLE_ID	"\x91\x5F\x9E\x4C"
#define MANUF_PARTTABLE_ID_SIZE	4
#define MANUF_DIAGS_OFFSET	0x82c
#define MANUF_BOOT2_OFFSET	0x830
#define MANUF_BOOTD_OFFSET	0x834
#define MANUF_FILES_OFFSET	0x838
#define MANUF_PTABLE_OFFSET	0x818
#define MANUF_PTABLE_ID		"\x91\x5F\x9E\x4C"
#define MANUF_CR4_OFFSET	0x81D

// page offsets for classic Nspire partitions, or older Nspire CX without a partition table
#define MANUF_PAGE_OFFSET	0x00000
#define BOOT2_PAGE_OFFSET	0x00020
#define BOOTD_PAGE_OFFSET	0x00A80
#define DIAGS_PAGE_OFFSET 	0x00B00
#define FILES_PAGE_OFFSET	0x01000
#define NDEND_PAGE_OFFSET	0x10000

#define CX2_MANUF_PAGE_OFFSET	0x00000
#define CX2_BOOTL_PAGE_OFFSET	0x00040
#define CX2_PTTDT_PAGE_OFFSET	0x00140
#define CX2_UNKN1_PAGE_OFFSET	0x00180
#define CX2_DEVCR_PAGE_OFFSET	0x001C0
#define CX2_OSLDR_PAGE_OFFSET	0x00200
#define CX2_INSTL_PAGE_OFFSET	0x002C0
#define CX2_OINST_PAGE_OFFSET	0x004C0
#define CX2_OSDAT_PAGE_OFFSET	0x006C0
#define CX2_DIAGS_PAGE_OFFSET	0x00740
#define CX2_UNKN2_PAGE_OFFSET	0x00880
#define CX2_OSYST_PAGE_OFFSET	0x00900
#define CX2_LOGIN_PAGE_OFFSET	0x01C80
#define CX2_FILES_PAGE_OFFSET	0x03240
#define CX2_NDEND_PAGE_OFFSET	0x10000
