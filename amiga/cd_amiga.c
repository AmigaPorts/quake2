#include "../client/client.h"

#ifndef __amigaos4__
#include "twfsound_cd.h"

static qboolean cdValid = false;
static qboolean cdPlaying = false;
static qboolean cdWasPlaying = false;
static qboolean cdInitialised = false;
static qboolean cdEnabled = false;
static byte     cdPlayTrack;
static byte     cdMaxTrack;
static int      cdEmulatedStart;
static int      cdEmulatedLength;

struct TWFCDData *twfdata=0;

cvar_t *cd_nocd = 0;
#endif

int CDAudio_GetAudioDiskInfo()
{
#ifndef __amigaos4__
  int err;
  cdValid = false;

  err=TWFCD_ReadTOC(twfdata);
  if(err==TWFCD_FAIL)
  {
	Com_Printf("CD Audio: Drive not ready\n");
	return(0);
  }

  cdMaxTrack=(twfdata->TWFCD_Table).cda_LastTrack;
  if ((twfdata->TWFCD_Table.cda_FirstAudio)==TWFCD_NOAUDIO)
  {
	Com_Printf("CD Audio: No music tracks\n");
	return(0);
  }
  cdValid = true;
  return(1);
#else
	Com_Printf("CD Audio not supported\n");
	return 0;
#endif
}

void CDAudio_Play(int track, qboolean looping)
{
#ifndef __amigaos4__
  int err;
  struct TagItem tags[]=
  {
	TWFCD_Track_Start,0,
	TWFCD_Track_End,0,
	TWFCD_Track_Count,0,
	TWFCD_Track_PlayMode,SCSI_CMD_PLAYAUDIO12,
	0,0
  };

  if (!cdEnabled) return;

  if ((track < 1) || (track > cdMaxTrack))
  {
	CDAudio_Stop();
	return;
  }

  if (!cdValid)
  {
	CDAudio_GetAudioDiskInfo();
	if (!cdValid) return;
  }

  if (!((twfdata->TWFCD_Table.cda_TrackData[track]).cdt_Audio))
  {
	Com_Printf("CD Audio: Track %i is not audio\n", track);
	return;
  }

  if(cdPlaying)
  {
	if(cdPlayTrack == track) return;
	CDAudio_Stop();
  }

  tags[0].ti_Data=track;
  tags[1].ti_Data=track;
  tags[2].ti_Data=1;
  tags[3].ti_Data=SCSI_CMD_PLAYAUDIO12;
  err=TWFCD_PlayTracks(twfdata,tags);
  if (err!=TWFCD_OK)
  {
	tags[3].ti_Data=SCSI_CMD_PLAYAUDIO_TRACKINDEX;
	err=TWFCD_PlayTracks(twfdata,tags);
	if (err!=TWFCD_OK)
	{
	  tags[3].ti_Data=SCSI_CMD_PLAYAUDIO10;
	  err=TWFCD_PlayTracks(twfdata,tags);
	}
  }

  if (err!=TWFCD_OK)
  {
	Com_Printf("CD Audio: CD PLAY failed\n");
	return;
  }
  cdEmulatedLength = (((twfdata->TWFCD_Table.cda_TrackData[track]).cdt_Length)/75)*1000;
  cdEmulatedStart = Sys_Milliseconds();

  cdPlayTrack = track;
  cdPlaying = true;
#endif
}


void CDAudio_Stop(void)
{
#ifndef __amigaos4__
  if (!cdEnabled) return;
  if (!cdPlaying) return;

  TWFCD_MotorControl(twfdata,TWFCD_MOTOR_STOP);
  cdWasPlaying = false;
  cdPlaying = false;
#endif
}


void CDAudio_Resume(void)
{
#ifndef __amigaos4__
  int err;
  if (!cdEnabled) return;
  if (!cdWasPlaying) return;
  if (!cdValid) return;

  err=TWFCD_PausePlay(twfdata);

  cdPlaying = true;
  if (err==TWFCD_FAIL) cdPlaying = false;
#endif
}

void CDAudio_Pause(void)
{
#ifndef __amigaos4__
	int err;

	if (!cdEnabled)
	return;

	if (!cdPlaying)
	return;

	err = TWFCD_PausePlay(twfdata);

	cdWasPlaying = cdPlaying;
	cdPlaying = false;
#endif
}


void CDAudio_Update(void)
{
#ifndef __amigaos4__
	if (cd_nocd && cd_nocd->modified)
	{
	cd_nocd->modified = 0;

	if (cd_nocd->value)
	    CDAudio_Pause();
	else
	    CDAudio_Resume();
	}
  if(Sys_Milliseconds() > (cdEmulatedStart + cdEmulatedLength))
  {
	cdPlaying = false;
	CDAudio_Play(cdPlayTrack, true);
  }
#endif
}


int CDAudio_HardwareInit()
{
#ifndef __amigaos4__
  char devname[255];
  char unitname[255];
  int unit;
  cdInitialised=true;
  if (cdInitialised)
  {
	if (0<(GetVar("env:Quake2/cd_device",devname,255,0)))
	{
	}
	else
	{
	  cdInitialised=0;
	return 0;
	}

	if (0<GetVar("env:Quake2/cd_unit",unitname,255,0))
	{
	  unit=atoi(unitname);
	}
	else
	{
	  cdInitialised=0;
	  return 0;
	}
  }
  if (cdInitialised)
  {
	char test[1024];
	sprintf(test,"%s %i ",devname,unit);
	if (twfdata=TWFCD_Setup(devname,unit))
	   return CDAudio_GetAudioDiskInfo();
	cdInitialised = false;
	Com_Printf("cdValidAudio: Init failed\n");
  }
#endif
  
  return 0;
}

int CDAudio_Init(void)
{
#ifndef __amigaos4__
  cdEnabled = false;
  cdInitialised = false;

  cd_nocd = Cvar_Get("cd_nocd", "0", CVAR_ARCHIVE);

  if (COM_CheckParm("-nocdaudio")) return -1;

  cdEnabled = true;

  if(CDAudio_HardwareInit())
  {
	cdEnabled=true;
	Com_Printf("CD Audio Initialized\n");
  }

#endif

  return(0);
}


void CDAudio_Shutdown(void)
{
#ifndef __amigaos4__
  if(!cdInitialised) return;
  CDAudio_Stop();
  TWFCD_Shutdown(twfdata);
#endif
}
