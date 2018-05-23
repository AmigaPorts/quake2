/*
**  snd_ahi.c
**
**  AHI sound driver
**
**  Written by Jarmo Laakkonen <jami.laakkonen@kolumbus.fi>
**  Adopted for Quake 2 Sound DLL by Hans-Joerg Frieden <Hans-JoergF@hyperion-entertainment.com>
**
**  TODO:
**  
**  - Add mode requester
*/

#include <exec/exec.h>
#include <devices/ahi.h>
#include <devices/timer.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <clib/alib_protos.h>
#include <proto/ahi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../client/client.h"
#include "../client/snd_loc.h"
#include "../amiga/snddll.h"

#ifndef __amigaos4__
#include "dll.h"
#endif

#ifdef __amigaos4__
ULONG callback(struct Hook *hook, struct AHIAudioCtrl *actrl, struct AHIEffChannelInfo *info)
{
  	hook->h_Data = (APTR)(info->ahieci_Offset[0]);
  	return 0;
}
#else
	// 68k code in symbol "callback"
	#include "snd_68k.h"
#endif

struct ChannelInfo
{
  struct AHIEffChannelInfo cinfo;
  ULONG x[1];
};

#ifdef __amigaos4__
struct AHIIFace* IAHI = NULL;
#endif

struct Library *AHIBase = NULL;
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static BYTE AHIDevice = -1;
static struct AHIAudioCtrl *actrl = NULL;
static ULONG rc = 1;
static struct ChannelInfo info;

static int speed;
static UBYTE *dmabuf = NULL;
static int buflen;

#define MINBUFFERSIZE 16384

sndimport_t si;

struct Hook EffHook = {
  0, 0,
  (HOOKFUNC)callback,
  0, 0,
};

void AHISND_Shutdown(dma_t *dma)
{
  if (actrl && IAHI) {
	info.cinfo.ahie_Effect = AHIET_CHANNELINFO | AHIET_CANCEL;
	AHI_SetEffect(&info, actrl);
	AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
  }

  if (rc == 0 && IAHI && actrl) {
	AHI_UnloadSound(0, actrl);
	rc = 1;
  }

  if (dmabuf) {
#if defined (__PPC__) && !defined(__amigaos4__)
	FreeVecPPC(dmabuf);
#else
	FreeVec(dmabuf);
#endif    
	dmabuf = NULL;
  }

  if (actrl && IAHI) {
	AHI_FreeAudio(actrl);
	actrl = NULL;
  }

#ifdef __amigaos4__
	if (IAHI)
	{
		DropInterface(IAHI);
		IAHI = NULL;
	}
#endif

  if (AHIDevice == 0) {
	CloseDevice((struct IORequest *)AHIio);
	AHIDevice = -1;
  }

  if (AHIio) {
	DeleteIORequest((struct IORequest *)AHIio);
	AHIio = NULL;
  }

  if (AHImp) {
	DeleteMsgPort(AHImp);
	AHImp = NULL;
  }
}

qboolean AHISND_Init(dma_t *dma)
{
  struct AHISampleInfo sample;
  int i;
  ULONG mixfreq, playsamples;
  UBYTE name[256];
  ULONG mode;
  ULONG type;
  int ahichannels;
  int ahibits;
  cvar_t *cv;

  info.cinfo.ahieci_Channels = 1;
  info.cinfo.ahieci_Func = &EffHook;
  info.cinfo.ahie_Effect = AHIET_CHANNELINFO;
	EffHook.h_Data = 0;

	cv = si.Cvar_Get("ahi_speed", "22", CVAR_ARCHIVE);
	if (cv->value == 22) speed = 22050;
	else if (cv->value == 11) speed = 11025;
	else speed = 11025;

	cv = si.Cvar_Get("ahi_channels", "2", CVAR_ARCHIVE);
	if (cv->value == 2) ahichannels = 2;
	else if (cv->value == 1) ahichannels = 1;
	else ahichannels = 2;

	cv = si.Cvar_Get("ahi_bits", "16", CVAR_ARCHIVE);
	if (cv->value == 16) ahibits = 16;
	else if (cv->value == 8) ahibits = 8;
	else ahibits = 16;

  if (ahibits != 8 && ahibits != 16)
	ahibits = 16;

  if (ahichannels == 1) {
	if (ahibits == 16) type = AHIST_M16S;
	else type = AHIST_M8S;
  } else {
	if (ahibits == 16) type = AHIST_S16S;
	else type = AHIST_S8S;
  }

  if ((AHImp = CreateMsgPort()) == NULL) {
	si.Con_Printf(PRINT_ALL, "ERROR: Can't create AHI message port\n");
	return false;
  }

  if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest))) == NULL) {
	si.Con_Printf(PRINT_ALL, "ERROR:Can't create AHI io request\n");
	return false;
  }

  AHIio->ahir_Version = 4;

  if ((AHIDevice = OpenDevice("ahi.device", AHI_NO_UNIT, (struct IORequest *)AHIio, 0)) != 0) {
	si.Con_Printf(PRINT_ALL, "Can't open ahi.device version 4\n");
	return false;
  }

  AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;

	if (!(IAHI = (struct AHIIFace*)GetInterface( AHIBase, "main", 1, NULL )))
	{
		return false;
	}

  if ((actrl = AHI_AllocAudio(AHIA_AudioID, AHI_DEFAULT_ID,
			      AHIA_MixFreq, speed,
			      AHIA_Channels, 1,
			      AHIA_Sounds, 1,
			      TAG_END)) == NULL) {
	si.Con_Printf(PRINT_ALL, "Can't allocate audio\n");
	return false;
  }

  AHI_GetAudioAttrs(AHI_INVALID_ID, actrl, AHIDB_MaxPlaySamples, &playsamples,
		    AHIDB_BufferLen, 256, AHIDB_Name, (ULONG)&name,
		    AHIDB_AudioID, &mode, TAG_END);
  AHI_ControlAudio(actrl, AHIC_MixFreq_Query, &mixfreq, TAG_END);
  buflen = playsamples * speed / mixfreq;

  if (buflen < MINBUFFERSIZE) buflen = MINBUFFERSIZE;

#if defined (__PPC__) && !defined(__amigaos4__)
  if ((dmabuf = AllocVecPPC(buflen, MEMF_ANY | MEMF_PUBLIC | MEMF_CLEAR, 0)) == NULL) {
#else
  if ((dmabuf = AllocVec(buflen, MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR)) == NULL) {
#endif
	si.Con_Printf(PRINT_ALL, "Can't allocate AHI dma buffer\n");
	return false;
  }

  dma->buffer = (unsigned char *)dmabuf;
  dma->channels = ahichannels;
  dma->speed = speed;
  dma->samplebits = ahibits;
  dma->samples = buflen / (dma->samplebits / 8);
  dma->submission_chunk = 1;
  dma->samplepos = 0;

  sample.ahisi_Type = type;
  sample.ahisi_Address = (APTR)dmabuf;
  sample.ahisi_Length = buflen / AHI_SampleFrameSize(type);

  if ((rc = AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, actrl)) != 0) {
	si.Con_Printf(PRINT_ALL, "Can't load sound\n");
	return false;
  }
 
  if (AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END) != 0) {
	si.Con_Printf(PRINT_ALL, "Can't start playback\n");
	return false;
  }

  si.Con_Printf(PRINT_ALL, "AHI audio initialized\n");
  si.Con_Printf(PRINT_ALL, "AHI mode: %s (%08x)\n", name, mode);
  si.Con_Printf(PRINT_ALL, "Output: %ibit %s\n", ahibits, ahichannels == 2 ? "stereo" : "mono");
  si.Con_Printf(PRINT_ALL, "snd_ahi.dll\nwritten by Jarmo Laakkonen and Hans-Joerg Frieden\n");

  AHI_Play(actrl, AHIP_BeginChannel, 0,
		  AHIP_Freq, speed,
		  AHIP_Vol, 0x10000,
		  AHIP_Pan, 0x8000,
		  AHIP_Sound, 0,
		  AHIP_EndChannel, NULL,
		  TAG_END);
  AHI_SetEffect(&info, actrl);
  return true;
}

int AHISND_GetDMAPos(dma_t *dma)
{
  return (dma->samplepos = (int)(EffHook.h_Data) * dma->channels);
}

/* ========== EXPORTED FUNCTIONS ================================================ */

sndexport_t GetSndAPI(sndimport_t simp)
{
	sndexport_t se;

	si = simp;

	se.api_version      = SND_API_VERSION;
	se.Init             = AHISND_Init;
	se.Shutdown         = AHISND_Shutdown;
	se.GetDMAPos        = AHISND_GetDMAPos;
	se.BeginPainting    = NULL;
	se.Submit           = NULL;

	return se;
}

#ifndef __amigaos4__

void*  dllFindResource(int id, char *pType)
{
	return NULL;
}

void* dllLoadResource(void *pHandle)
{
	return NULL;
}

void dllFreeResource(void *pHandle)
{
	return;
}

dll_tExportSymbol DLL_ExportSymbols[] =
{
	{dllFindResource,"dllFindResource"},
	{dllLoadResource,"dllLoadResource"},
	{dllFreeResource,"dllFreeResource"},
	{GetSndAPI,"GetSndAPI"},
	{0,0},
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
	{0,0,0,0}
};

int DLL_Init(void)
{
	return 1;
}

void DLL_DeInit(void)
{
}
#endif

