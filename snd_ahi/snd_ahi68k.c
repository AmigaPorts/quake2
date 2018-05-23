/*
**  snd_ahi68k.c
**
**  AHI sample position routine
**
**  Written by Jarmo Laakkonen <jami.laakkonen@kolumbus.fi>
**
*/

#include <devices/ahi.h>
#include <utility/hooks.h>
#define REG(r, x) x __asm( #r )

#ifdef __PPC__
ULONG ASM EffFunc(REG(a0, struct Hook *hook), REG(a2, struct AHIAudioCtrl *actrl),
		 REG(a1, struct AHIEffChannelInfo *info))
#else
ULONG EffFunc(__REGA0(struct Hook *hook), __REGA2(struct AHIAudioCtrl *actrl),
		 __REGA1(struct AHIEffChannelInfo *info))
#endif
{
  extern int ahi_pos;

  hook->h_Data = (APTR)(info->ahieci_Offset[0]);
  return 0;
}
