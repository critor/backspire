#include <os.h>
#include "types.h"
#include "ndlessext.h"
#include "nand.h"
#include "screen.h"
#include "utils.h"
#include "charmaps.h"
#include "console.h"

// classic Nspire & older Nspire CX partition table
static const u32 classic_parts_pages_offsets[CLASSIC_CX_NPARTS+1]={MANUF_PAGE_OFFSET,BOOT2_PAGE_OFFSET,BOOTD_PAGE_OFFSET,DIAGS_PAGE_OFFSET,FILES_PAGE_OFFSET,0};

// Nspire CX2 partition table
static const u32 cx2_parts_pages_offsets[CX2_NPARTS+1]={CX2_MANUF_PAGE_OFFSET,CX2_BOOTL_PAGE_OFFSET,CX2_PTTDT_PAGE_OFFSET,CX2_UNKN1_PAGE_OFFSET,CX2_DEVCR_PAGE_OFFSET,CX2_OSLDR_PAGE_OFFSET,CX2_INSTL_PAGE_OFFSET,CX2_OINST_PAGE_OFFSET,CX2_OSDAT_PAGE_OFFSET,CX2_DIAGS_PAGE_OFFSET,CX2_UNKN2_PAGE_OFFSET,CX2_OSYST_PAGE_OFFSET,CX2_LOGIN_PAGE_OFFSET,CX2_FILES_PAGE_OFFSET,0};

// classic Nspire & older Nspire CX boot partitions
static const char* classic_bootmodes[2]={"Boot2","Diags"};

// Nspire CX2 boot partitions
static const char* cx2_bootmodes[3]={"OSLoader","Installer","Diags"};

extern u16 sscreen[SCREEN_PIXELS];

#define LINETEXT_SIZE 128

static void dispBytesNum(u16 x, u16 y, u32 val,char* suffix, int unit, int n) {
  setBytesColor();
  dispNum(x,y,val,suffix,unit,n);
  resetCurColor();
}

static void dispPagesNum(u16 x, u16 y, u32 val, char* suffix, int unit, int n) {
  setPagesColor();
  dispNum(x,y,val,suffix,unit,n);
  resetCurColor();
}

static void dispBlocksNum(u16 x, u16 y, u32 val, char* suffix, int unit, int n) {
  setBlocksColor();
  dispNum(x,y,val,suffix,unit,n);
  resetCurColor();
}

static void dispPartitionsNum(u16 x, u16 y, u8 val) {
  char txt[3] = {0};
  sprintf(txt, "%02d", val);
  setCurColorRGB(0,0,0);
  drwBufStr(x,y,txt,0,0,0);
  resetCurColor();
}

static void dispVersionStr(char* txt, int flags) {
  setCurColorRGB(0,0xFF,0x7F);
  disp(txt,flags);
  resetCurColor();
}

static void dispKey(t_key key, char* txt) {
  if(!isKeyPressed(key)) setCurColorRGB(0xFF,0xFF,0xFF);
  else setCurColorRGB(0,0xFF,0xFF);
  disp(txt,0);
  if(!strcmp(txt,"<") || !strcmp(txt,">")) {
    setConsoleCol(getConsoleCol()-1);
    disp("-",I_TRANSP);
  }
  else if(!strcmp(txt,"^") || !strcmp(txt,"v")) {
    setConsoleCol(getConsoleCol()-1);
    disp("|",I_TRANSP);
  }
  resetCurColor();
}

static void sprintVersion(char* txt, const u32* version_ptr) {
  u8 major = *(((u8*)version_ptr)+3);
  u8 minor = *(((u8*)version_ptr)+2);
  u16 build = *((u16*)version_ptr);
  sprintf(txt,"%d.%d.%d.%d",major,(int)(minor/10),minor%10,build);
}

enum {ACTION_NONE, ACTION_LEFT, ACTION_RIGHT, ACTION_UP, ACTION_DOWN, ACTION_TAB, ACTION_DEL, ACTION_REBOOT, ACTION_ESC};

static int horizRate(int val, int max) {
  return 1+val*(SCREEN_WIDTH-3)/max;
}

int main(int argc, char** argv) {
  u8 action = ACTION_NONE;
  u8* buffer = malloc(MANUF_FILES_OFFSET);
  u8 nparts, idownpart;
  u8 nboots;
  u32 size, offset;
  u32* parts_pages_offsets;
  char** bootmodes;
  char txt[LINETEXT_SIZE];
  u8* ptr=0;
  u8* last_ptr;
  u8 signature[4];
  u8 i;
  u8 ok=1;
  u32* minos_ptr;
  u32* boot_ptr;
  u32* last_minos_ptr;
  u8 type = getHardwareType();
  u8 field = 0;
  u8 nfields = 2;

  initScreen();
  startScreen();
  convertRGB565(sscreen,SCR_320x240_565);

  while(action!=ACTION_ESC) {
    if (type == NS_CX2) { // CX2
      memcpy(signature, "DATA", 4);
      nparts=CX2_NPARTS;
      parts_pages_offsets = cx2_parts_pages_offsets;
      idownpart = 8;
      nboots = 3;
      bootmodes = cx2_bootmodes;
    }
    else {
      memcpy(signature, "\xAA\xC6\x8C\x92", 4);
      nparts = CLASSIC_CX_NPARTS;
      parts_pages_offsets = classic_parts_pages_offsets;
      char* buffer=malloc(MANUF_FILES_OFFSET+4);
      ext_read_nand(buffer,MANUF_FILES_OFFSET+4,0,0,0,NULL);
      idownpart = 2;
      nboots = 2;
      bootmodes = classic_bootmodes;

      // TI-Nspire CX/CM partition table
      if(!memcmp(buffer+MANUF_PTABLE_OFFSET,MANUF_PTABLE_ID,strlen(MANUF_PTABLE_ID))) {
        const long int offsets_offsets[CLASSIC_CX_NPARTS]={0,MANUF_BOOT2_OFFSET,MANUF_BOOTD_OFFSET,MANUF_DIAGS_OFFSET,MANUF_FILES_OFFSET};
        for(i=1;i<nparts;i++)
          parts_pages_offsets[i] = *((long int*)(buffer+offsets_offsets[i]))/NAND_PAGE_SIZE;
      }
      else { // classic
        for(i=0;i<nparts;i++)
          parts_pages_offsets[i] = classic_parts_pages_offsets[i];
      }
    }
    parts_pages_offsets[nparts] = NAND_SIZE/NAND_PAGE_SIZE;
    offset = parts_pages_offsets[idownpart]*NAND_PAGE_SIZE;
    size = (parts_pages_offsets[idownpart+1]-parts_pages_offsets[idownpart])*NAND_PAGE_SIZE;
    buffer = realloc(buffer, size);
    ext_read_nand(buffer, size, offset, 0, 0, NULL);
    ok = 0;
    for(last_ptr = buffer + size - NAND_PAGE_SIZE; (type == NS_CX2)?last_ptr>buffer:last_ptr>=buffer; last_ptr -= NAND_PAGE_SIZE) {
      ok = !memcmp(last_ptr, signature, 4);
      if(ok) break;
    }
    if(ptr < buffer || ptr > last_ptr) ptr = last_ptr;
    minos_ptr = ptr+(type==NS_CX2?0xC:4);
    boot_ptr = ptr+(type==NS_CX2?4:0x10);
    last_minos_ptr = last_ptr+(type==NS_CX2?0xC:4);

    clrBuf(getScreen());
    memcpy(getOffScreen(),sscreen,SCREEN_SIZE);

    resetConsole();
    resetCurColor();
    displn("       backSpire 1.1.1",I_TRANSP|I_BOLD);

    resetConsole();
    for(i=0;i<5;i++) displn("",0);
    disp("                                              ",I_TRANSP);
    switch(type) {
      case NS_CX2:
        strcpy(txt,"CX II");
        break;
      case NS_CX:
        strcpy(txt,"CX");
        break;
      case NS_CM:
        strcpy(txt,"CM");
        break;
      case NS_CL:
        strcpy(txt,"classic");
        break;
      case NS_OTHER:
        strcpy(txt,"unknown");
        break;
    }
    setCurColorRGB(0xFF,0x7F,0);
    disp(txt,I_TRANSP|I_BOLD);

    resetConsole();
    for(i=0;i<1;i++) displn("",0);
    if(ok) {
      if(*last_minos_ptr) {
        sprintVersion(txt, last_minos_ptr);
        setCurColorRGB(0xFF,0,0);
        disp("OS < ",I_TRANSP);
        dispVersionStr(txt,0);
        setCurColorRGB(0xFF,0,0);
        displn(" forbidden",I_TRANSP);
      }
      else {
        setCurColorRGB(0,0x7F,0);
        displn("Any OS allowed",I_TRANSP);
      }
    }
    else displn("",I_TRANSP);

    if(ok) {
      resetConsole();
      for(i=0;i<2;i++) displn("",I_TRANSP);
      setInactiveColor();
      displn("To install an older OS:",I_TRANSP|I_BOLD);
      if(*last_minos_ptr) displn("- Patch minOS on active page",I_TRANSP|I_BOLD);
      if(type==NS_CX2) {
        displn("- Reboot into maintenance menu",I_TRANSP|I_BOLD);
        displn("- Delete the current OS",I_TRANSP|I_BOLD);
        displn("- Reboot once completed",I_TRANSP|I_BOLD);
        displn("- Send the older OS",I_TRANSP|I_BOLD);
      }
      else {
        displn("- Exit this program",I_TRANSP|I_BOLD);
        displn("- Send the older OS",I_TRANSP|I_BOLD);
      }
    }

    setInactiveColor();
    u16 y0 = 9*CHAR_HEIGHT;
    u16 y = y0+2;
    u16 lmargin = 2;
    u16 x = SCREEN_WIDTH/2-4*CHAR_WIDTH;
    u16 x1 = 7*CHAR_WIDTH;
    u16 x2 = x+7*CHAR_WIDTH;
    drwBufStr(lmargin,y,  "      Flash NAND",0,1,0);
    drwBufStr(x+lmargin,y,"      BootData partition",0,1,0);
    y += CHAR_HEIGHT+1;
    drwBufHoriz(y,0,SCREEN_WIDTH-1);
    y+=1;
    drwBufStr(lmargin,y,"parts:",0,1,0);
    dispPartitionsNum(x1,y,nparts);
    setInactiveColor();
    drwBufStr(x+lmargin,y,"part :",0,1,0);
    dispPartitionsNum(x2,y,idownpart);
    y += CHAR_HEIGHT;
    setInactiveColor();
    drwBufStr(lmargin,y,"size :",0,1,0);
    dispBlocksNum(x1,y,parts_pages_offsets[nparts]/(NAND_BLOCK_SIZE/NAND_PAGE_SIZE),"blocks",1,3);
    setInactiveColor();
    drwBufStr(x+lmargin,y,"start:",0,1,0);
    dispBlocksNum(x2,y,offset/NAND_BLOCK_SIZE,"b",1,3);
    dispPagesNum(x2+6*CHAR_WIDTH,y,offset/NAND_PAGE_SIZE,"p",1,5);
    dispBytesNum(x2+14*CHAR_WIDTH,y,offset,"B",1,7);
    y += CHAR_HEIGHT;
    dispPagesNum(x1-2*CHAR_WIDTH,y,parts_pages_offsets[nparts],"pages",1,5);
    setInactiveColor();
    drwBufStr(x+lmargin,y,"end  :",0,1,0);
    dispBlocksNum(x2,y,(offset+size)/NAND_BLOCK_SIZE-1,"b",1,3);
    dispPagesNum(x2+6*CHAR_WIDTH,y,(offset+size)/NAND_PAGE_SIZE-1,"p",1,5);
    dispBytesNum(x2+14*CHAR_WIDTH,y,offset+size-1,"B",1,7);
    y += CHAR_HEIGHT;
    dispBytesNum(x1-4*CHAR_WIDTH,y,parts_pages_offsets[nparts]*NAND_PAGE_SIZE,"Bytes",1,7);
    setInactiveColor();
    drwBufStr(x+lmargin,y,"size :",0,1,0);
    dispBlocksNum(x2,y,size/NAND_BLOCK_SIZE,"b",1,3);
    dispPagesNum(x2+6*CHAR_WIDTH,y,size/NAND_PAGE_SIZE,"p",1,5);
    dispBytesNum(x2+14*CHAR_WIDTH,y,size,"B",1,7);
    y += CHAR_HEIGHT;
    setBlocksColor();
    invertCurColorRGB();
    drwBufStr(lmargin,y,"block=",0,1,0);
    dispPagesNum(lmargin+6*CHAR_WIDTH,y,NAND_BLOCK_SIZE/NAND_PAGE_SIZE,"p",1,2);
    setPagesColor();
    invertCurColorRGB();
    drwBufStr(lmargin+11*CHAR_WIDTH,y,"page=",0,1,0);
    dispBytesNum(lmargin+16*CHAR_WIDTH,y,NAND_PAGE_SIZE,"B",1,3);
    setInactiveColor();
    drwBufStr(x+lmargin,y,"used :",0,1,0);
    dispPagesNum(x2,y,(last_ptr-buffer)/NAND_PAGE_SIZE+ok,"/",0,5);
    dispPagesNum(x2+6*CHAR_WIDTH,y,size/NAND_PAGE_SIZE,"pages",1,5);
    y += CHAR_HEIGHT;
    setInactiveColor();
    drwBufBox(0,y0,SCREEN_WIDTH-1,y);
    drwBufVert(x,y0,y);
    
    resetConsole();
    resetCurColor();
    for(i=0;i<18;i++) displn("",I_TRANSP);

    if(!ok) {
      setCurColorRGB(0xFF,0,0);
      displn("Unsupported partition content !",I_TRANSP);
      resetCurColor();
      displn("Press any key to exit...",I_TRANSP);
      showScreen();
      while(any_key_pressed());
      while(!any_key_pressed());
      while(any_key_pressed());
      break;
    }

    resetConsole();
    for(i=0;i<18;i++) displn("",0);

    y0=getConsoleRow()*CHAR_HEIGHT-CHAR_HEIGHT-4;
    y=y0+CHAR_HEIGHT+3;

    drwBufBox(0,y0,SCREEN_WIDTH-1,y);
    u8* tptr;
    for(tptr=buffer;tptr<buffer+size;tptr+=NAND_PAGE_SIZE) {
      if(!memcmp(tptr,signature,4))
        setCurColorRGB(0,0xFF,0);
      else
        setCurColorRGB(0xBF,0xBF,0xBF);
      drawBufFullBox(horizRate(tptr-buffer,size),y0+1,horizRate(tptr-buffer+NAND_PAGE_SIZE,size),y-1);
    }

    if(ptr==last_ptr) setCurColorRGB(0xFF,0,0);
    else {
      setPagesColor();
      invertCurColorRGB();
    }
    drawBufFullBox(horizRate(ptr-buffer,size),y0,horizRate(ptr+NAND_PAGE_SIZE-buffer,size)-1,y);
    drwBufStr(lmargin,y0+2,"BootData page",0,1,0);

    disp("page : ", I_TRANSP);
    dispPagesNum(getConsoleCol()*CHAR_WIDTH,getConsoleRow()*CHAR_HEIGHT,(ptr-buffer)/NAND_PAGE_SIZE,0,1,5);
    if(ptr!=last_ptr) {
      setPagesColor();
      invertCurColorRGB();
    }
    else {
      setCurColorRGB(0xFF,0,0);
    }
    disp("       (",I_TRANSP);
    if(ptr!=last_ptr) {
      disp("older - changes ignored)",I_TRANSP);
    }
    else {
      disp("active - changes effective)",I_TRANSP);
    }
    resetCurColor();
    displn("",I_TRANSP);
    disp("minOS: ",I_TRANSP|(field==0?I_BOLD:0));
    sprintVersion(txt, minos_ptr);
    dispVersionStr(txt,0);
    displn("",I_TRANSP);
    disp("boot : ",I_TRANSP|(field==1?I_BOLD:0));
    u32 bootmode = *boot_ptr;
    if(bootmode>=nboots) bootmode = nboots-1;
    disp(bootmodes[bootmode],I_TRANSP);
    resetCurColor();

    action = ACTION_NONE;
    while(action==ACTION_NONE || (action!=ACTION_LEFT && action!=ACTION_RIGHT && any_key_pressed())) {
      static const u32 nullVersion = 0;
      resetConsole();
      for(i=0;i<21;i++) displn("",I_TRANSP);
      drwBufHoriz(getConsoleRow()*CHAR_HEIGHT-1,0,SCREEN_WIDTH-1);
      dispKey(KEY_NSPIRE_TAB,"tab");
      disp(": patch minimum allowed OS version to ",I_TRANSP);
      sprintVersion(txt, &nullVersion);
      dispVersionStr(txt,0);
      displn("",I_TRANSP);
      if(is_touchpad) {
        dispKey(KEY_NSPIRE_DOC,"doc");
        disp(" + ",I_TRANSP);
        dispKey(KEY_NSPIRE_ENTER,"enter");
        disp(" + ",I_TRANSP);
        dispKey(KEY_NSPIRE_EE,"EE");
      }
      else {
        dispKey(KEY_NSPIRE_HOME,"home");
        disp(" + ",I_TRANSP);
        dispKey(KEY_NSPIRE_ENTER,"enter");
        disp(" + ",I_TRANSP);
        dispKey(KEY_NSPIRE_P,"P");
      }
      displn(": reboot into maintenance menu (hold)",I_TRANSP);
      dispKey(KEY_NSPIRE_ESC,"esc");
      disp(": exit     ",I_TRANSP);
      dispKey(KEY_NSPIRE_LEFT,"<");
      dispKey(KEY_NSPIRE_RIGHT,">");
      disp(": prev/next page  ",I_TRANSP);
      dispKey(KEY_NSPIRE_UP,"^");
      dispKey(KEY_NSPIRE_DOWN,"v");
      displn(": prev/next field",I_TRANSP);
      showScreen();

      if(isKeyPressed(KEY_NSPIRE_ESC)) action=ACTION_ESC;
      else if(isKeyPressed(KEY_NSPIRE_TAB)) action=ACTION_TAB;
      else if(isKeyPressed(KEY_NSPIRE_LEFT)) action=ACTION_LEFT;
      else if(isKeyPressed(KEY_NSPIRE_RIGHT)) action=ACTION_RIGHT;
      else if(isKeyPressed(KEY_NSPIRE_UP)) action=ACTION_UP;
      else if(isKeyPressed(KEY_NSPIRE_DOWN)) action=ACTION_DOWN;
      else if(isKeyPressed(KEY_NSPIRE_DEL)) action=ACTION_DEL;

      if( (is_touchpad && isKeyPressed(KEY_NSPIRE_DOC)  && isKeyPressed(KEY_NSPIRE_ENTER) && isKeyPressed(KEY_NSPIRE_EE))
      || (!is_touchpad && isKeyPressed(KEY_NSPIRE_HOME) && isKeyPressed(KEY_NSPIRE_ENTER) && isKeyPressed(KEY_NSPIRE_P))) {
        action = ACTION_REBOOT;
        break;
      }
    }

    if(action==ACTION_LEFT) {
      if(ptr > buffer && (type < NS_CX2 || ptr - NAND_PAGE_SIZE > buffer)) ptr -= NAND_PAGE_SIZE;
    }
    else if(action==ACTION_RIGHT) {
      if(ptr < last_ptr) ptr += NAND_PAGE_SIZE;
    }
    if(action==ACTION_UP) {
      if(field>0) field--;
    }
    else if(action==ACTION_DOWN) {
      if(field+1<nfields) field++;
    }
    else if(action==ACTION_REBOOT) trigger_reset();
    else if(action==ACTION_TAB || action==ACTION_DEL) {
      if(action==ACTION_TAB) {
        if(field==0) {
          *minos_ptr = 0;
        }
        else if(field==1) {
          *boot_ptr=(*boot_ptr)+1;
          if(*boot_ptr>=nboots) *boot_ptr=0;
        }
      }
      else if(action==ACTION_DEL) {
        memset(ptr,0xFF,NAND_PAGE_SIZE);
        ptr -= NAND_PAGE_SIZE;
      }
      ext_erase_nand(offset, offset+size-1);
      ext_write_nand(buffer, size, offset);
    }

  }
  stopScreen();
  free(buffer);
  return 0;
}
