/* $XConsortium: XF86_Mono.c,v 1.1 94/10/05 13:34:15 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/XF86_Mono.c,v 3.4 1994/09/07 15:51:42 dawes Exp $ */
#include "X.h"
#include "os.h"

#define _NO_XF86_PROTOTYPES

#include "xf86.h"
#include "xf86_Config.h"

extern ScrnInfoRec vga2InfoRec;
extern ScrnInfoRec monoInfoRec;

#ifdef BUILD_VGA2
#define SCREEN0 &vga2InfoRec
#else
#define SCREEN0 NULL
#endif

#ifdef BUILD_VGA2_MONO
#define SCREEN1 &monoInfoRec
#else
#define SCREEN1 NULL
#endif

ScrnInfoPtr xf86Screens[] = 
{
  SCREEN0,
  SCREEN1,
};

int  xf86MaxScreens = sizeof(xf86Screens) / sizeof(ScrnInfoPtr);

int xf86ScreenNames[] =
{
  VGA2,
  MONO,
  -1
};

#ifdef BUILD_VGA2
int vga2ValidTokens[] =
{
  STATICGRAY,
  CHIPSET,
  CLOCKS,
  MODES,
  SCREENNO,
  OPTION,
  VIDEORAM,
  VIEWPORT,
  VIRTUAL,
  CLOCKPROG,
  BIOSBASE,
  BLACK,
  WHITE,
  -1
};
#endif

#ifdef BUILD_VGA2_MONO
int monoValidTokens[] =
{
  STATICGRAY,
  CHIPSET,
  OPTION,
  MEMBASE,
  SCREENNO,
  VIRTUAL,
  VIEWPORT,
  -1
};
#endif

#define MONO_SERVER
#include "xf86ExtInit.h"

