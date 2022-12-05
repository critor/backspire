#include <os.h>
#include "types.h"

#define max(a,b) (((a)<(b))?(b):(a))
#define min(a,b) (((a)<(b))?(a):(b))

void wait(int timesec);
void trigger_reset(void);

enum {NS_CL, NS_CX, NS_CM, NS_CX2, NS_OTHER};
u8 getHardwareType(void);

void resetCurColor(void);
void setBlocksColor(void);
void setPagesColor(void);
void setBytesColor(void);
void setInactiveColor(void);

void dispNumStr(u16 x, u16 y, char* txt, int unit);
void dispNum(u16 x, u16 y, u32 val,char* suffix, int unit, int n);
