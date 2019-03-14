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

#ifndef SDL_SYSRENDER_HXX
#define SDL_SYSRENDER_HXX

#include "SDL_lib.hxx"


typedef struct SDL_RenderDriver SDL_RenderDriver;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct SDL_Texture
{
  Uint32 format;
  int access;

  int w;
  int h;

  SDL_Renderer* renderer;

  void* pixels;
  Uint32 pitch;

  SDL_Texture* prev;
  SDL_Texture* next;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct SDL_Renderer
{
  SDL_Texture* textures;
  SDL_Texture* target;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct SDL_RenderDriver
{
};

#endif
