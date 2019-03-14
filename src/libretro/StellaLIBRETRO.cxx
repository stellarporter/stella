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
#include "StellaLIBRETRO.hxx"

#include "AtariNTSC.hxx"
#include "AudioSettings.hxx"
#include "FrameBuffer.hxx"
#include "Serializer.hxx"
#include "Sound.hxx"
#include "StateManager.hxx"
#include "Switches.hxx"
#include "TIA.hxx"
#include "TIASurface.hxx"

#include "SDL_sysrender.hxx"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StellaLIBRETRO::StellaLIBRETRO()
{
  /*
  1048 samples ~ 30 frames
   620 samples ~ 50 frames
   524 samples ~ 60 frames
  */

  audio_buffer = make_unique<Int16[]>(audio_buffer_max);

  console_timing = ConsoleTiming::ntsc;
  console_format = "AUTO";

  video_aspect_ntsc = 0;
  video_aspect_pal = 0;

  video_palette = "standard";
  video_filter = NTSCFilter::PRESET_OFF;
  video_ready = false;

  audio_samples = 0;
  audio_mode = "byrom";

  video_phosphor = "byrom";
  video_phosphor_blend = 60;
  
  rom_image = make_unique<Uint8[]>(getROMMax());

  system_ready = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool StellaLIBRETRO::create(bool logging)
{
  system_ready = false;

  FilesystemNode rom("rom");

  // auto-detect properties
  destroy();

  myOSystem = make_unique<OSystemLIBRETRO>();
  myOSystem->create();

  myOSystem->settings().setValue("format", console_format);

  if(myOSystem->createConsole(rom) != EmptyString)
    return false;


  // auto-detect settings
  console_timing = myOSystem->console().timing();
  phosphor_default = myOSystem->frameBuffer().tiaSurface().phosphorEnabled();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // build play system
  destroy();

  myOSystem = make_unique<OSystemLIBRETRO>();
  myOSystem->create();

  Settings& settings = myOSystem->settings();

  if(logging)
  {
    settings.setValue("loglevel", 999);
    settings.setValue("logtoconsole", true);
  }

  settings.setValue("speed", 1.0);
  settings.setValue("uimessages", false);

  settings.setValue("format", console_format);
  settings.setValue("palette", video_palette);

  settings.setValue("tia.zoom", 1);
  settings.setValue("tia.inter", false);
  settings.setValue("tia.aspectn", 100);
  settings.setValue("tia.aspectp", 100);

  //fastscbios
  // Fast loading of Supercharger BIOS

  settings.setValue("tv.filter", video_filter);

  settings.setValue("tv.phosphor", video_phosphor);
  settings.setValue("tv.phosblend", video_phosphor_blend);

  settings.setValue(AudioSettings::SETTING_PRESET, static_cast<int>(AudioSettings::Preset::custom));
  settings.setValue(AudioSettings::SETTING_SAMPLE_RATE, getAudioRate());
  settings.setValue(AudioSettings::SETTING_FRAGMENT_SIZE, 2);       // minimum half-frames
  settings.setValue(AudioSettings::SETTING_BUFFER_SIZE, 2048 / 2);  // xsqueeker = 1240 half-frames
  settings.setValue(AudioSettings::SETTING_HEADROOM, 0);
  settings.setValue(AudioSettings::SETTING_RESAMPLING_QUALITY, static_cast<int>(AudioSettings::ResamplingQuality::nearestNeightbour));
  settings.setValue(AudioSettings::SETTING_VOLUME, 100);
  settings.setValue(AudioSettings::SETTING_STEREO, audio_mode);

  if(myOSystem->createConsole(rom) != EmptyString)
    return false;


  if(video_phosphor == "never") setVideoPhosphor(1, video_phosphor_blend);


  video_ready = false;
  audio_samples = 0;

  system_ready = true;
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::destroy()
{
  system_ready = false;

  video_ready = false;
  audio_samples = 0;

  myOSystem.reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::runFrame()
{
  // poll input right at vsync
  updateInput();

  // run vblank routine and draw frame
  updateVideo();

  // drain generated audio
  updateAudio();

  // give user time to respond
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::updateInput()
{
  Console& console = myOSystem->console();

  console.controller(Controller::Left).update();
  console.controller(Controller::Right).update();

  console.switches().update();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::updateVideo()
{
  TIA& tia = myOSystem->console().tia();

  while(1)
  {
    Uint32 beam_x, beam_y;

    tia.updateScanline();

    tia.electronBeamPos(beam_x, beam_y);
    if(!beam_y) break;
  }

  video_ready = tia.newFramePending();
  if(video_ready)
  {
    FrameBuffer& frame = myOSystem->frameBuffer();

    tia.renderToFrameBuffer();
    frame.updateInEmulationMode(0);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::updateAudio()
{
  audio_samples = myOSystem->sound().queueSize() * myOSystem->audioSettings().fragmentSize() * 2;
  //printf("updateAudio - %d %d\n", myOSystem->sound().queueSize(), myOSystem->audioSettings().fragmentSize());

  if(!audio_samples) return;


  static unique_ptr<float[]> audio_buffer_f = make_unique<float[]>(audio_buffer_max);

  audio_samples = SDL_DequeueAudio(1, audio_buffer_f.get(), audio_samples * sizeof(float)) / sizeof(float);

  for(int lcv = 0; lcv < audio_samples; lcv++)
    audio_buffer[lcv] = audio_buffer_f[lcv] * static_cast<float>(0x7fff);


  if(myOSystem->sound().queueSize())
    printf("updateAudio stop - ** failure ** - %d %d\n", myOSystem->sound().queueSize(), myOSystem->audioSettings().fragmentSize());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool StellaLIBRETRO::loadState(const void* data, Uint32 size)
{
  Serializer state;

  state.putByteArray((const uInt8*) data, size);

  if(!myOSystem->state().loadState(state))
    return false;

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool StellaLIBRETRO::saveState(void* data, Uint32 size)
{
  Serializer state;

  if(!myOSystem->state().saveState(state))
    return false;

  if(state.length() > size)
    return false;

  state.getByteArray((uInt8*) data, state.length());
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
float StellaLIBRETRO::getVideoAspect()
{
  Uint32 width = myOSystem->console().tia().width() * 2;
  float par;

  if(getVideoNTSC())
  {
    if(!video_aspect_ntsc)
      // non-interlace square pixel clock -- 1.0 pixel @ color burst -- double-width pixels
      par = (6.1363635 / 3.579545454) / 2;
    else
      par = video_aspect_ntsc / 100.0;
  }
  else
  {
    if(!video_aspect_pal)
      // non-interlace square pixel clock -- 0.8 pixel @ color burst -- double-width pixels
      par = (7.3750000 / (4.43361875 * 4/5)) / 2;
    else
      par = video_aspect_pal / 100.0;
  }

  // display aspect ratio
  return (width * par) / getVideoHeight();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void* StellaLIBRETRO::getVideoBuffer()
{
  SDL_Texture* texture = SDL_GetRenderTarget((SDL_Renderer*) 1);

  return texture->pixels;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool StellaLIBRETRO::getVideoNTSC()
{
  const ConsoleInfo& console_info = myOSystem->console().about();
  string format = console_info.DisplayFormat;

  return (format == "NTSC") || (format == "NTSC*") ||
         (format == "PAL60") || (format == "PAL60*") ||
         (format == "SECAM60") || (format == "SECAM60*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool StellaLIBRETRO::getVideoResize()
{
  if(render_width != getRenderWidth() || render_height != getRenderHeight())
  {
    render_width = getRenderWidth();
    render_height = getRenderHeight();

    return true;
  }

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setROM(const void* data, Uint32 size)
{
  memcpy(rom_image.get(), data, size);

  rom_size = size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setConsoleFormat(Uint32 mode)
{
  switch(mode)
  {
  case 0: console_format = "AUTO"; break;
  case 1: console_format = "NTSC"; break;
  case 2: console_format = "PAL"; break;
  case 3: console_format = "SECAM"; break;
  case 4: console_format = "NTSC50"; break;
  case 5: console_format = "PAL60"; break;
  case 6: console_format = "SECAM60"; break;
  }

  if(system_ready)
    myOSystem->settings().setValue("format", console_format);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setVideoFilter(Uint32 mode)
{
  switch(mode)
  {
  case 0: video_filter = NTSCFilter::PRESET_OFF; break;
  case 1: video_filter = NTSCFilter::PRESET_COMPOSITE; break;
  case 2: video_filter = NTSCFilter::PRESET_SVIDEO; break;
  case 3: video_filter = NTSCFilter::PRESET_RGB; break;
  case 4: video_filter = NTSCFilter::PRESET_BAD; break;
  case 5: video_filter = NTSCFilter::PRESET_CUSTOM; break;
  }

  if(system_ready)
  {
    myOSystem->settings().setValue("tv.filter", static_cast<int>(video_filter));
    myOSystem->frameBuffer().tiaSurface().setNTSC(video_filter);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setVideoPalette(Uint32 mode)
{
  switch(mode)
  {
  case 0: video_palette = "standard"; break;
  case 1: video_palette = "z26"; break;
  case 2: video_palette = "custom"; break;
  }

  if(system_ready)
  {
    myOSystem->settings().setValue("palette", video_palette);
    myOSystem->console().setPalette(video_palette);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setVideoPhosphor(Uint32 mode, Uint32 blend)
{
  switch(mode)
  {
  case 0: video_phosphor = "byrom"; break;
  case 1: video_phosphor = "never"; break;
  case 2: video_phosphor = "always"; break;
  }

  video_phosphor_blend = blend;

  if(system_ready)
  {
    myOSystem->settings().setValue("tv.phosphor", video_phosphor);
    myOSystem->settings().setValue("tv.phosblend", blend);

    switch(mode)
	{
    case 0: myOSystem->frameBuffer().tiaSurface().enablePhosphor(phosphor_default, blend); break;
    case 1: myOSystem->frameBuffer().tiaSurface().enablePhosphor(false, blend); break;
    case 2: myOSystem->frameBuffer().tiaSurface().enablePhosphor(true, blend); break;
	}
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StellaLIBRETRO::setAudioStereo(int mode)
{
  switch(mode)
  {
  case 0: audio_mode = "byrom"; break;
  case 1: audio_mode = "mono"; break;
  case 2: audio_mode = "stereo"; break;
  }

  if(system_ready)
  {
    myOSystem->settings().setValue(AudioSettings::SETTING_STEREO, audio_mode);
	myOSystem->console().initializeAudio();
  }
}
