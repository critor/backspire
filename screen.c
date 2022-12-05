#include <os.h>
#include "types.h"
#include "utils.h"
#include "screen.h"
#include "charmaps.h"
#include "nand.h"
#include "ndlessext.h"
#define SCREEN_CONTRAST_ADDR 0x900F0020

#define SCREEN_BASE_PTR		0xC0000010 
#define SCREEN_MODE_ADDR	0xC000001C
#define SCREEN_INT_ADDR		0xC0000020

u16* SCREEN_BASE_ADDR = 0;
u16* orig_screen=0;
u16* malloc_screen=0;
u16 offscreen[SCREEN_WIDTH*SCREEN_HEIGHT];
//u8 offscreen[SCREEN_SIZE];
int contrast=0;
u8 screen_type = SCR_320x240_565;
u16 curcolor=0;

u16* getScreen()
{	return SCREEN_BASE_ADDR;
}

u16* getOffScreen()
{	return offscreen;
}

void setScreen(u16* buf)
{	
	SCREEN_BASE_ADDR=buf;
	*(volatile unsigned*) SCREEN_BASE_PTR = buf;
}

u8 getScreenType() {
	if(has_colors) {
		if(nl_ndless_rev() >= 2004) { // Ndless 4.2+
			if(lcd_type()==SCR_240x320_565)
				return SCR_240x320_565;
		}
		else { // Ndless < 4.2
			char f=0;
			ext_read_nand(&f, 1, MANUF_CR4_OFFSET, 0, 0, 0);
			if(f==1)
				return SCR_240x320_565;
		}
	}
	return SCR_320x240_565;
}

void initScreen()
{
	screen_type=getScreenType();
	setCurColorRGB(0,0,0);
	SCREEN_BASE_ADDR	=*(u16**)SCREEN_BASE_PTR;
	orig_screen = SCREEN_BASE_ADDR;
	contrast=*(volatile unsigned*) SCREEN_CONTRAST_ADDR;
}


u8 off_mode=0;
unsigned int setContrast(unsigned int level)
{ 	unsigned int lowlevel = level &0b11111111;
	if(lowlevel>CONTRAST_MAX) lowlevel=CONTRAST_MAX;
	if(lowlevel<CONTRAST_MIN) lowlevel=CONTRAST_MIN;
	level = (level & ~0b11111111)|lowlevel;
	contrast=level;
	if(!has_colors || !off_mode)
		*(volatile unsigned*) SCREEN_CONTRAST_ADDR=level;
	return level;
}

void switchScrOffOn(u8 s)
{
	if(!has_colors)
	{
		int mask = 0b100000000001;
		int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		if(s)	mode |= mask;
		else	mode &= ~mask;
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
	//	return mode&mask;
	}
	else
	{	if(s) off_mode=0;
		else off_mode=1;
		if(!off_mode)
			setContrast(contrast);
		else
			*(volatile unsigned*) SCREEN_CONTRAST_ADDR=(contrast&~0b11111111)|CONTRAST_MIN;
	}
}

void startScreen()
{
	if(!has_colors) {
		int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		mode = mode&~0b1110;
		mode = mode|0b1000;
		switchScrOffOn(0); // prevents displaying garbage when the screen mode will be changed
		if(!malloc_screen) {
			malloc_screen=(u16*) malloc(SCREEN_SIZE);
			clrBuf(malloc_screen);
			setScreen(malloc_screen);
		}
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
		switchScrOffOn(1);
	}
}



void stopScreen()
{	u16* screen=SCREEN_BASE_ADDR;
	if(!has_colors)
	{	int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		mode = mode&~0b100001110;
		mode = mode|0b0100;
		switchScrOffOn(0); // prevents displaying garbage when the screen mode will be changed
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
		setScreen(orig_screen);
		if(malloc_screen) {
			free(malloc_screen);
			malloc_screen=0;
		}
		clrBuf(SCREEN_BASE_ADDR);
		switchScrOffOn(1);
	}
	else {
		if(screen!=orig_screen) {
			memcpy(orig_screen,screen,SCREEN_SIZE);
			setScreen(orig_screen);
		}
	}
}

void showScreen() {
  memcpy(getScreen(),offscreen,SCREEN_SIZE);
}

void setPixel(u16 x, u16  y, u16 color)
{	if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{	
		if(screen_type==SCR_320x240_565) {
			offscreen[y*SCREEN_WIDTH+x]=color;
		}
		else {
			offscreen[x*SCREEN_HEIGHT+y]=color;			
		}
	}
}

void setPixelRGB(u16 x, u16 y, u8 r, u8 g, u8 b)
{	if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{	
		u16 color;
		if(has_colors) color=rgb565(r,g,b);
		else	  color=rgb2gs(r,g,b);
		if(screen_type==SCR_320x240_565) {
			offscreen[y*SCREEN_WIDTH+x]=color;
		}
		else {
			offscreen[x*SCREEN_HEIGHT+y]=color;			
		}
	}
}

void convertRGB565toGS(u16* sscreen, u16  w, u16  h) {
	u16  i,j;
	u16* ptr=sscreen;
	u16 val;
	u8 r,g,b;
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			val=*ptr;
			b=(val&0b11111)<<3;
			g=(val&0b11111100000)>>3;
			r=(val&0b1111100000000000)>>8;
			*ptr=rgb2gs(r,g,b);
			ptr++;
		}
}

void convertRGB565320to240(u16* buffer) {
	u16  i,j;
	u16* ptr=buffer;
	u16* tmpbuffer=malloc(SCREEN_SIZE);
	u16 val;
	for(j=0;j<SCREEN_HEIGHT;j++)
		for(i=0;i<SCREEN_WIDTH;i++) {
			val=*ptr;
			tmpbuffer[i*SCREEN_HEIGHT+j]=val;
			ptr++;
		}
	memcpy(buffer,tmpbuffer,SCREEN_SIZE);
	free(tmpbuffer);
}

void convertRGB565(u16* buffer, u8 buf_type)
{
	if(!has_colors)
		convertRGB565toGS(buffer,SCREEN_WIDTH,SCREEN_HEIGHT);
	else if(buf_type!=screen_type)
		convertRGB565320to240(buffer);		
}

void setCurColorRGB(u8 r,u8 g,u8 b) {
	if(!has_colors)
		curcolor=rgb2gs(r,g,b);
	else
		curcolor=rgb565(r,g,b);
}

void invertCurColorRGB() {
  curcolor=~curcolor;
}

void putChar(u16 x, u16 y, char ch, u8 trsp, u8 bold)
{
  u8 i, j;
  u8 pixelOn;
  for(i = 0; i < CHAR_HEIGHT; i++) {
    for(j = 0; j < CHAR_WIDTH; j++) {
      pixelOn = _font_bits[(u8)ch][i];
      pixelOn = pixelOn<<j;
      pixelOn = pixelOn & 0x80 ;  	
      if (pixelOn) {
        setPixel(x + j, y + i, curcolor);
        if(bold) setPixel(x + j - 1, y + i, curcolor);
      } else if(!trsp) {
        setPixel(x + j, y + i, ~curcolor);
      } 
    }
  }
  if(!trsp && x>0) {
    for (i=0;i<CHAR_HEIGHT;i++) {
      setPixel(x - 1, y + i, ~curcolor);
    }
  }
}

void drwBufStr(u16 x, u16 y, char* str, u8 ret, u8 trsp, u8 bold)
{
  uint32_t l = strlen(str);
  uint32_t i;
  u8 stop=0;
  for (i = 0; i < l && !stop; i++) {
    if (str[i] == 0x0A) {
      if(ret)
      { x = 0;
        y += CHAR_HEIGHT;
      }
      else
      { putChar(x,y, ' ', trsp, bold); 
        x += CHAR_WIDTH;
      }
    } else {
      putChar(x, y, str[i], trsp, bold);
      x += CHAR_WIDTH;
    }
    if (x >= SCREEN_WIDTH-CHAR_WIDTH)
    { if(ret)
      { x = 0;
        y += CHAR_HEIGHT;
      }
      else
        stop=1;
    }
  }
}

void dispBufImgRGB(u16* buf, int16_t xoff, int16_t yoff, u8* img, u16 width, u16 height,u8 border)
{
	u16 dwidth=width, dheight=height;
	u16 data_x=0, data_y=0;
	u16 x = 0, y = 0;
	float i, j;
	if(xoff < 0){
		dwidth = dwidth + xoff;
		data_x = (int)(-xoff);
		xoff = 0;
	}
	if(yoff < 0){
		dheight = dheight + yoff;
		data_y = (int)(-yoff);	
		yoff = 0;
	}
	u8 r,g,b;
	for(i=0, x=0; (int)i < dwidth && x < SCREEN_WIDTH; i+= 1, x++)
		for(j=0, y=0; (int)j < dheight && y < SCREEN_HEIGHT; j+= 1, y++)
		{	b=img[(((int)j+data_y)*width+(int)i+data_x)*3];
			g=img[(((int)j+data_y)*width+(int)i+data_x)*3+1];
			r=img[(((int)j+data_y)*width+(int)i+data_x)*3+2];
			if(!border || b!=0 || g!=0 || r!=0)
				setPixelRGB(xoff + x, yoff + dheight-1-y, r, g, b);
		}
}

void clrBuf(u16* buf)
{
	if(has_colors || malloc_screen)
		clrBufBox(offscreen,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	else
		memset(SCREEN_BASE_ADDR,0,SCREEN_PIXELS/2);
}

void clrBufBox(u16* buf, u16 x, u16 y, u16 w, u16 h) {
	u16 color=~curcolor;
	u16 i,j;
	for(j=0;j<h;j++)
		for(i=0;i<w;i++)
			setPixel(x+i,y+j,color);
}

void drwBufHoriz(u16 y, u16 x1, u16 x2)
{
	u16 m = max(x1,x2);
	u16 i = min(x1,x2);
	while(i<=m)
	{	setPixel(i,y,curcolor);
		i++;
	}
}

void drwBufVert(u16 x, u16 y1, u16 y2)
{
	u16 m = max(y1,y2);
	u16 i = min(y1,y2);
	while(i<=m)
	{	setPixel(x,i,curcolor);
		i++;
	}
}

void drwBufBox(u16 x1, u16 y1, u16 x2, u16 y2) {
	drwBufHoriz(y1,x1,x2);
	drwBufHoriz(y2,x1,x2);
	drwBufVert(x1,y1,y2);
	drwBufVert(x2,y1,y2);
}

void drawBufFullBox(u16 x1, u16 y1, u16 x2, u16 y2) {
	u16 m = max(y1,y2);
	u16 i = min(y1,y2);
	while(i<=m)
	{	drwBufHoriz(i,x1,x2);
		i++;
	}
}
