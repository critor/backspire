#include "types.h"

#define SCREEN_PIXELS	(SCREEN_WIDTH*SCREEN_HEIGHT)
#define SCREEN_SIZE		(SCREEN_PIXELS*2)
#define CONTRAST_MIN	0x60
#define CONTRAST_MAX	0xC0

#define rgb2gs(r,g,b)	(((~((((r)*30+(g)*59+(b)*11)/100)>>4))&0b1111)<<1)
#define rgb565(r,g,b)	((((r)<<8)&0xf800)|(((g)<<3)&0x07e0)|((b)>>3))

u8 getScreenType();

void convertRGB565toGS(u16* sscreen, u16 w, u16 h);
void convertRGB565320to240(u16* buffer);
void convertRGB565(u16* buffer, u8 buf_type);
void setCurColorRGB(u8 r,u8 g,u8 b);
void invertCurColorRGB();

void setPixel(u16 x, u16  y, u16 color);
void setPixelRGB(u16 x, u16 y, u8 r, u8 g, u8 b);

void initScreen();
void showScreen();
void startScreen();
void stopScreen();

void setScreen(u16* buf);
u16* getScreen();
u16* getOffScreen();

void switchScrOffOn(u8 s);
unsigned int setContrast(unsigned int level);

void putChar(u16 x, u16 y, char ch, u8 trsp, u8 bold);
void drwBufStr(u16 x, u16 y, char* str, u8 ret, u8 trsp, u8 bold);

void dispBufImgRGB(u16* buf, int16_t xoff, int16_t yoff, u8* img, u16 width, u16 height,u8 border);

void drwBufHoriz(u16 y, u16 x1, u16 x2);
void drwBufVert(u16 x, u16 y1, u16 y2);

void drwBufBox(u16 x1, u16 y1, u16 x2, u16 y2);
void drawBufFullBox(u16 x1, u16 y1, u16 x2, u16 y2);

void clrBuf(u16* buf);
void clrBufBox(u16* buf, u16 x, u16 y, u16 w, u16 h);
