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

#ifndef STELLA_LIBRETRO_HXX
#define STELLA_LIBRETRO_HXX

#include "OSystemLIBRETRO.hxx"

#include "Console.hxx"
#include "ConsoleTiming.hxx"
#include "EmulationTiming.hxx"
#include "EventHandler.hxx"
#include "M6532.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "TIASurface.hxx"


/**
  This class wraps Stella core for easier libretro maintenance
*/
class StellaLIBRETRO
{
  public:
    StellaLIBRETRO();
    virtual ~StellaLIBRETRO() = default;

  public:
    OSystemLIBRETRO& osystem() { return *myOSystem; }

    bool create(bool logging);
    void destroy();
    void reset() { myOSystem->console().system().reset(); }

    void runFrame();

    bool loadState(const void* data, Uint32 size);
    bool saveState(void* data, Uint32 size);

  public:
    const char* getCoreName() { return "Stella"; }
    const char* getROMExtensions() { return "a26|bin"; }

    void*  getROM() { return rom_image.get(); }
    Uint32 getROMSize() { return rom_size; }
    Uint32 getROMMax() { return 512 * 1024; }

    Uint8* getRAM() { return myOSystem->console().system().m6532().getRAM(); }
    Uint32 getRAMSize() { return 128; }

    Uint32 getStateMax() { return 0x1000; }

    bool   getConsoleNTSC() { return console_timing == ConsoleTiming::ntsc; }

    float  getVideoAspect();
    bool   getVideoNTSC();
    float  getVideoRate() { return getVideoNTSC() ? 60.0 : 50.0; }

    bool   getVideoReady() { return video_ready; }
    Uint32 getVideoZoom() { return myOSystem->frameBuffer().tiaSurface().ntscEnabled() ? 2 : 1; }
    bool   getVideoResize();

    void*  getVideoBuffer();
    Uint32 getVideoWidth() { return getVideoZoom()==1 ? myOSystem->console().tia().width() : getVideoWidthMax(); }
    Uint32 getVideoHeight() { return myOSystem->console().tia().height(); }
    Uint32 getVideoPitch() { return getVideoWidthMax() * 4; }

    Uint32 getVideoWidthMax() { return AtariNTSC::outWidth(160); }
    Uint32 getVideoHeightMax() { return 312; }

    Uint32 getRenderWidth() { return getVideoZoom()==1 ? myOSystem->console().tia().width() * 2 : getVideoWidthMax(); }
    Uint32 getRenderHeight() { return myOSystem->console().tia().height() * getVideoZoom(); }

    float  getAudioRate() { return getConsoleNTSC() ? (262 * 76 * 60) / 38.0 : (312 * 76 * 50) / 38.0; }
    bool   getAudioReady() { return audio_samples > 0; }
    Uint32 getAudioSize() { return audio_samples / 2; }

    Int16* getAudioBuffer() { return audio_buffer.get(); }

  public:
    void   setROM(const void* data, Uint32 size);

    void   setConsoleFormat(Uint32 mode);

    void   setVideoAspectNTSC(Uint32 value) { video_aspect_ntsc = value; };
    void   setVideoAspectPAL(Uint32 value) { video_aspect_pal = value; };

    void   setVideoFilter(Uint32 mode);
    void   setVideoPalette(Uint32 mode);
    void   setVideoPhosphor(Uint32 mode, Uint32 blend);

    void   setAudioStereo(int mode);

    void   setInputEvent(Event::Type type, Int32 state) { myOSystem->eventHandler().handleEvent(type, state); }

  protected:
    void   updateInput();
    void   updateVideo();
    void   updateAudio();

  private:
    // Following constructors and assignment operators not supported
    StellaLIBRETRO(const StellaLIBRETRO&) = delete;
    StellaLIBRETRO(StellaLIBRETRO&&) = delete;
    StellaLIBRETRO& operator=(const StellaLIBRETRO&) = delete;
    StellaLIBRETRO& operator=(StellaLIBRETRO&&) = delete;

    unique_ptr<OSystemLIBRETRO> myOSystem;
    Uint32 system_ready;

    unique_ptr<Uint8[]> rom_image;
    Uint32 rom_size;

    ConsoleTiming console_timing;
    string console_format;

    Uint32 render_width, render_height;

    bool video_ready;

    unique_ptr<Int16[]> audio_buffer;
    Uint32 audio_samples;

    static const Uint32 audio_buffer_max = 2048 * 2;

  private:
    string video_palette;
    string video_phosphor;
    Uint32 video_phosphor_blend;

    Uint32 video_aspect_ntsc;
    Uint32 video_aspect_pal;
    NTSCFilter::Preset video_filter;

    string audio_mode;

    bool phosphor_default;
};

#endif
