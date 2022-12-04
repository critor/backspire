#include "types.h"

#define SCREEN_PIXELS	(SCREEN_WIDTH*SCREEN_HEIGHT)
#define SCREEN_SIZE		(SCREEN_PIXELS*2)
#define CONTRAST_MIN	0x60
#define CONTRAST_MAX	0xC0

#define rgb2gs(r,g,b)	(((~((((r)*30+(g)*59+(b)*11)/100)>>4))&0b1111)<<1)
#define rgb565(r,g,b)	((((r)<<8)&0xf800)|(((g)<<3)&0x07e0)|((b)>>3))

unsigned int getContrast();
void drwBufStr(u16 x, u16 y, char* str, u8 ret, u8 trsp, u8 bold);
void putBufChar(u16 x, u16 y, char ch, u8 trsp, u8 bold);
void setBufPixel(u16 x, u16  y, u16 color);
