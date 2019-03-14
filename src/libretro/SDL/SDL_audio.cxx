//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2018 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "SDL_lib.hxx"
#include "bspf.hxx"


static SDL_AudioSpec sdl_audiospec;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_AudioDeviceID SDLCALL SDL_OpenAudioDevice(
  const char* device, int iscapture, const SDL_AudioSpec* desired,
  SDL_AudioSpec* obtained, int allowed_changes)
{
  memcpy(&sdl_audiospec, desired, sizeof(SDL_AudioSpec));
  memcpy(obtained, desired, sizeof(SDL_AudioSpec));

  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC Uint32 SDLCALL SDL_DequeueAudio(SDL_AudioDeviceID dev, void *data, Uint32 len)
{
  sdl_audiospec.callback(sdl_audiospec.userdata, (Uint8*) data, len);

  return len;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_AudioStatus SDLCALL SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev)
{
  return SDL_AUDIO_PLAYING;
}
