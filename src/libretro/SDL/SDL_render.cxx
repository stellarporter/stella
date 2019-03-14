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

#include "SDL_sysrender.hxx"


static SDL_Renderer sdl_renderer;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
  return &sdl_renderer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_DestroyRenderer(SDL_Renderer * renderer)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_RenderClear(SDL_Renderer * renderer)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_RenderPresent(SDL_Renderer * renderer)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_RenderCopy(SDL_Renderer * renderer,
                                    SDL_Texture * texture,
                                    const SDL_Rect * srcrect,
                                    const SDL_Rect * dstrect)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                                          Uint32 format, void *pixels, int pitch)
{
  return -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetRendererInfo(SDL_Renderer * renderer, SDL_RendererInfo * info)
{
  info->name = "software";
  info->num_texture_formats = 1;
  info->max_texture_width = 8192;
  info->max_texture_height = 8192;

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetRenderDriverInfo(int index, SDL_RendererInfo * info)
{
  info->name = "software";
  info->num_texture_formats = 1;
  info->max_texture_width = 8192;
  info->max_texture_height = 8192;

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_GetNumRenderDrivers(void)
{
  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Texture * SDLCALL SDL_CreateTexture(SDL_Renderer * renderer, Uint32 format,
                                                 int access, int w, int h)
{
  SDL_Texture* texture = new SDL_Texture;

  texture->w = w;
  texture->h = h;
  texture->format = format;
  texture->access = access;

  Uint32 size;

  switch(format)
  {
  case SDL_PIXELFORMAT_ARGB8888:
    size = 4;
    break;
	
  default:
    size = 1;

    printf("SDL_CreateTexture - missing pixel format %d\n", format);
    break;
  }

  texture->pitch = w * size;
  texture->renderer = renderer;

  return texture;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC void SDLCALL SDL_DestroyTexture(SDL_Texture * texture)
{
  if(texture) delete texture;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_UpdateTexture(SDL_Texture * texture, const SDL_Rect * rect,
                                       const void *pixels, int pitch)
{
  //printf("SDL_UpdateTexture %X %d - %X %X %d %d %d %d\n", pixels, pitch, texture, texture->pixels, texture->w, texture->h, texture->pitch, texture->access);

  texture->pixels = const_cast<void *>(pixels);

  // Stella framebuffer hack
  if(texture->w == 565 && texture->h == 320 && texture->access == SDL_TEXTUREACCESS_STREAMING)
    sdl_renderer.target = texture;

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_SetTextureBlendMode(SDL_Texture * texture, SDL_BlendMode blendMode)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC int SDLCALL SDL_SetTextureAlphaMod(SDL_Texture * texture, Uint8 alpha)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DECLSPEC SDL_Texture * SDLCALL SDL_GetRenderTarget(SDL_Renderer *renderer)
{
  if(renderer != (SDL_Renderer *) 1)
    return renderer->target;

  return sdl_renderer.target;
}
