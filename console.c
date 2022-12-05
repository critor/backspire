#include <os.h>
#include "screen.h"
#include "console.h"
#include "utils.h"
#include "charmaps.h"

int col=0;
int line=0;

int getConsoleCol() {
  return col;
}

int getConsoleRow() {
  return line;
}

void setConsoleCol(int newcol) {
  col = newcol;
}

void setConsoleRow(int newline) {
  line = newline;
}

void dispBuf(char* message, int params)
{ int ret = params & I_AUTORET;
  int trsp = params & I_TRANSP;
  int bold = params & I_BOLD;
  int l = strlen(message);
  int i, stop=0;
  for (i = 0; i < l && !stop; i++) {
    if (message[i] == 0x0A) {
      if ( ret )
      { col = 0;
        line ++;
      }
      else
      { putChar(col*CHAR_WIDTH, line*CHAR_HEIGHT, ' ', trsp, bold);
        col++;
      }
    } else {
      putChar(col*CHAR_WIDTH, line*CHAR_HEIGHT, message[i], trsp, bold);
      col ++;
    }
    if (col >= MAX_COL)
    { if ( !ret ) stop=1;
      else
      { col = 0;
        line ++;
      }
    }
    if(line>=MAX_LGN) { line=0; }
  }
}

void disp(char* msg, int params)
{	dispBuf(msg,params);
}

void displnBuf( unsigned char* buf, char* message, int params)
{	dispBuf(message, params);
	col=0;
	line++;
	if(line>=MAX_LGN) { line=0; }
}

void displn(char* msg, int params)
{	displnBuf(getScreen(),msg,params);
}

void resetConsole()
{	col=0;
	line=0;
}