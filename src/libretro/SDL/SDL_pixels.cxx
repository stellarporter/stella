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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC Uint32 SDL_MapRGB(const SDL_PixelFormat * format, Uint8 r, Uint8 g, Uint8 b)
{
  // X8R8G8B8
  return (r<<16) | (g<<8) | b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDL_GetRGB(Uint32 pixel, const SDL_PixelFormat * format,
                         Uint8 * r, Uint8 * g, Uint8 * b)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_PixelFormat * SDLCALL SDL_AllocFormat(Uint32 pixel_format)
{
  SDL_PixelFormat* format = new SDL_PixelFormat;

  format->format = pixel_format;
  format->palette = nullptr;

  switch(pixel_format)
  {
  case SDL_PIXELFORMAT_ARGB8888:
    format->BitsPerPixel = 32;
    format->BytesPerPixel = 4;
    break;

  default:
    format->BitsPerPixel = 8;
    format->BytesPerPixel = 1;
	
    printf("SDL_AllocFormat - missing pixel format %d\n", pixel_format);
    break;
  }

  return format;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_FreeFormat(SDL_PixelFormat *format)
{
  if(format) delete format;
}
