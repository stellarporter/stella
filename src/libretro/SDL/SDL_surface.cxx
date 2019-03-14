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

#include <stdio.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                                   Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
  //printf("SDL_CreateRGBSurface = %d %d %d\n", width, height, depth);

  depth /= 8;  

  SDL_Surface* surface = new SDL_Surface;

  surface->w = width;
  surface->h = height;
  surface->pitch = width * depth;
  surface->pixels = new Uint8[width * height * depth];

  return surface;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBSurfaceFrom(void *pixels,
                                                       int width, int height, int depth, int pitch,
                                                       Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
  depth /= 8;  

  SDL_Surface* surface = new SDL_Surface;

  surface->w = width;
  surface->h = height;
  surface->pitch = width * depth;
  surface->pixels = new Uint8[width * height * depth];

  return surface;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_FreeSurface(SDL_Surface * surface)
{
  if(surface)
  {
    if(surface->pixels) delete[] surface->pixels;
    delete surface;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_FillRect(SDL_Surface * dst, const SDL_Rect * rect, Uint32 color)
{
  return 0;
}
