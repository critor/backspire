#define MAX_COL SCREEN_WIDTH/CHAR_WIDTH
#define MAX_LGN SCREEN_HEIGHT/CHAR_HEIGHT
#define LSEPARATOR "----------------------------------------"

#define I_AUTORET 1
#define I_TRANSP 2
#define I_BOLD 4


void dispBuf(char* message, int params);
void disp(char* msg, int params);
void displnBuf( unsigned char* buf, char* message, int params);
void displn(char* msg, int params);
void resetConsole();
int getConsoleCol();
int getConsoleRow();
void setConsoleCol(int col);
void setConsoleRow(int line);

