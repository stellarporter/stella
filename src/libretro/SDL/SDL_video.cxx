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

#include "SDL_sysvideo.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_GetWindowSize(SDL_Window * window, int *w, int *h)
{
  *w = 1920;
  *h = 1080;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_SetWindowIcon(SDL_Window * window, SDL_Surface * icon)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_SetWindowTitle(SDL_Window * window, const char *title)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Window * SDLCALL SDL_CreateWindow(const char *title,
                                               int x, int y, int w,
                                               int h, Uint32 flags)
{
  SDL_Window* window = new SDL_Window;

  return window;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_DestroyWindow(SDL_Window * window)
{
  if(window) delete window;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetWindowDisplayIndex(SDL_Window * window)
{
  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
  mode->format = SDL_PIXELFORMAT_ARGB8888;
  mode->w = 1920;
  mode->h = 1080;

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetNumVideoDisplays(void)
{
  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC const char *SDLCALL SDL_GetCurrentVideoDriver(void)
{
  return "software";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect * rect)
{
  rect->x = 0;
  rect->y = 0;
  rect->w = 1920;
  rect->h = 1080;

  return 0;
}
