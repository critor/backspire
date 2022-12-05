#include "utils.h"
#include "screen.h"
#include "charmaps.h"

void wait(int timesec) {
    volatile long int i;
    for(i=0;i<timesec*10000;i++) {}
}

void trigger_reset() {
    *(volatile unsigned*)0x900A0008 = 2;
    *(unsigned *) 0x90140020=0x80;
}

u8 getHardwareType() {
    if(!hwtype()) return NS_CL;
    else {
        u8 type = nl_hwsubtype();
        switch(type) {
            case 0: return NS_CX;
            case 1: return NS_CM;
            case 2: return NS_CX2;
            default: return NS_OTHER;
        }
    }
}

void resetCurColor() {
    setCurColorRGB(0,0,0);
}

void setBlocksColor() {
    setCurColorRGB(0xFF,0xFF,0x7F);
}

void setPagesColor() {
    setCurColorRGB(0xFF,0xFF,0);
}

void setBytesColor() {
    setCurColorRGB(0xBF,0xBF,0);
}

void setInactiveColor() {
    setCurColorRGB(0x4F,0x4F,0x4F);
}

void dispNumStr(u16 x, u16 y, char* txt, int unit) {
    drwBufStr(x,y,txt,0,0,0);
    if(unit) drwBufStr(x+strlen(txt)*CHAR_WIDTH,y,"h",0,0,0);
}

void dispNum(u16 x, u16 y, u32 val,char* suffix, int unit, int n) {
    if(n<=0) n=7;
    char* txt = calloc(n+1, 1);
    snprintf(txt, n+1, "%0*lX", n, val);
    dispNumStr(x, y, txt, unit);
    if(suffix) {
        invertCurColorRGB();
        drwBufStr(x+(n+unit)*CHAR_WIDTH+1,y,suffix,0,1,0);
    }
    free(txt);
}
