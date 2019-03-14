#ifndef _MSC_VER
#include <stdbool.h>
#include <sched.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "libretro.h"

#include "StellaLIBRETRO.hxx"
#include "Event.hxx"
#include "Version.hxx"


static StellaLIBRETRO stella;

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static struct retro_system_av_info g_av_info;

// libretro UI settings
static int setting_ntsc, setting_pal;
static int setting_stereo, setting_filter, setting_palette;
static int setting_phosphor, setting_console, setting_phosphor_blend;

static bool system_reset;

static unsigned input_devices[2];

// TODO input:
// https://github.com/libretro/blueMSX-libretro/blob/master/libretro.c
// https://github.com/libretro/libretro-o2em/blob/master/libretro.c


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint32_t libretro_read_rom(void* data)
{
  memcpy(data, stella.getROM(), stella.getROMSize());

  return stella.getROMSize();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void update_input()
{
  if(!input_poll_cb) return;
  input_poll_cb();


#define EVENT stella.setInputEvent

  EVENT(Event::JoystickZeroUp, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP));
  EVENT(Event::JoystickZeroDown, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN));
  EVENT(Event::JoystickZeroLeft, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT));
  EVENT(Event::JoystickZeroRight, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT));
  EVENT(Event::JoystickZeroFire, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B));

  EVENT(Event::JoystickOneUp, input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP));
  EVENT(Event::JoystickOneDown, input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN));
  EVENT(Event::JoystickOneLeft, input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT));
  EVENT(Event::JoystickOneRight, input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT));
  EVENT(Event::JoystickOneFire, input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B));

  EVENT(Event::ConsoleLeftDiffA, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L));
  EVENT(Event::ConsoleLeftDiffB, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2));
  EVENT(Event::ConsoleColor, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3));
  EVENT(Event::ConsoleRightDiffA, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R));
  EVENT(Event::ConsoleRightDiffB, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2));
  EVENT(Event::ConsoleBlackWhite, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3));
  EVENT(Event::ConsoleSelect, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT));
  EVENT(Event::ConsoleReset, input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START));
  
#undef EVENT
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void update_geometry()
{
  struct retro_system_av_info av_info;

  retro_get_system_av_info(&av_info);

  environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void update_system_av()
{
  struct retro_system_av_info av_info;

  retro_get_system_av_info(&av_info);

  environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &av_info);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void update_variables(bool init = false)
{
  bool geometry_update = false;

  char key[256];
  struct retro_variable var;

#define RETRO_GET(x) \
  var.key = x; \
  var.value = NULL; \
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))

  RETRO_GET("stella_filter")
  {
    int value = 0;

    if(!strcmp(var.value, "disabled")) value = 0;
    else if(!strcmp(var.value, "composite")) value = 1;
    else if(!strcmp(var.value, "s-video")) value = 2;
    else if(!strcmp(var.value, "rgb")) value = 3;
    else if(!strcmp(var.value, "badly adjusted")) value = 4;

    if(setting_filter != value)
    {
      stella.setVideoFilter(value);

      geometry_update = true;
      setting_filter = value;
    }
  }

  RETRO_GET("stella_ntsc_aspect")
  {
    int value = 0;

    if(!strcmp(var.value, "auto")) value = 0;
    else value = atoi(var.value);

    if(setting_ntsc != value)
    {
      stella.setVideoAspectNTSC(value);

      geometry_update = true;
      setting_ntsc = value;
    }
  }

  RETRO_GET("stella_pal_aspect")
  {
    int value = 0;

    if(!strcmp(var.value, "auto")) value = 0;
    else value = atoi(var.value);

    if(setting_pal != value)
    {
      stella.setVideoAspectPAL(value);

      setting_pal = value;
      geometry_update = true;
    }
  }

  RETRO_GET("stella_palette")
  {
    int value = 0;

    if(!strcmp(var.value, "standard")) value = 0;
    else if(!strcmp(var.value, "z26")) value = 1;

    if(setting_palette != value)
    {
      stella.setVideoPalette(value);

      setting_palette = value;
    }
  }

  RETRO_GET("stella_console")
  {
    int value = 0;

    if(!strcmp(var.value, "auto")) value = 0;
    else if(!strcmp(var.value, "ntsc")) value = 1;
    else if(!strcmp(var.value, "pal")) value = 2;
    else if(!strcmp(var.value, "secam")) value = 3;
    else if(!strcmp(var.value, "ntsc50")) value = 4;
    else if(!strcmp(var.value, "pal60")) value = 5;
    else if(!strcmp(var.value, "secam60")) value = 6;

    if(setting_console != value)
    {
      stella.setConsoleFormat(value);

      setting_console = value;
	  system_reset = true;
    }
  }

  RETRO_GET("stella_stereo")
  {
    int value = 0;

    if(!strcmp(var.value, "auto")) value = 0;
    else if(!strcmp(var.value, "off")) value = 1;
    else if(!strcmp(var.value, "on")) value = 2;

    if(setting_stereo != value)
    {
      stella.setAudioStereo(value);

      setting_stereo = value;
    }
  }

  RETRO_GET("stella_phosphor")
  {
    int value = 0;

    if(!strcmp(var.value, "auto")) value = 0;
    else if(!strcmp(var.value, "off")) value = 1;
    else if(!strcmp(var.value, "on")) value = 2;

    if(setting_phosphor != value)
    {
      stella.setVideoPhosphor(value, setting_phosphor_blend);

      setting_phosphor = value;
    }
  }

  RETRO_GET("stella_phosphor_blend")
  {
    int value = 0;

    value = atoi(var.value);

    if(setting_phosphor_blend != value)
    {
      stella.setVideoPhosphor(setting_phosphor, value);

      setting_phosphor_blend = value;
    }
  }

  if(!init && !system_reset)
  {
    if(geometry_update) update_geometry();
  }

#undef RETRO_GET
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static bool reset_system()
{
  // clean restart
  stella.destroy();

  // apply libretro settings first
  update_variables(true);

  // start system
  if(!stella.create(log_cb ? true : false)) return false;

  // reset libretro window
  update_geometry();

  system_reset = false;
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
unsigned retro_api_version() { return RETRO_API_VERSION; }
unsigned retro_get_region() { return stella.getVideoNTSC() ? RETRO_REGION_NTSC : RETRO_REGION_PAL; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_get_system_info(struct retro_system_info *info)
{
  memset(info,0,sizeof(retro_system_info));

  info->library_name = stella.getCoreName();
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
  info->library_version = STELLA_VERSION GIT_VERSION;
  info->valid_extensions = stella.getROMExtensions();
  info->need_fullpath = false;
  info->block_extract = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_get_system_av_info(struct retro_system_av_info *info)
{
  memset(info,0,sizeof(retro_system_av_info));

  info->timing.fps            = stella.getVideoRate();
  info->timing.sample_rate    = stella.getAudioRate();

  info->geometry.base_width   = stella.getRenderWidth();
  info->geometry.base_height  = stella.getRenderHeight();

  info->geometry.max_width    = stella.getVideoWidthMax();
  info->geometry.max_height   = stella.getVideoHeightMax();

  info->geometry.aspect_ratio = stella.getVideoAspect();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_set_controller_port_device(unsigned port, unsigned device)
{
  if(port < 2)
  {
    switch (device)
    {
    case RETRO_DEVICE_NONE:
      input_devices[port] = RETRO_DEVICE_NONE;
      break;

    case RETRO_DEVICE_JOYPAD:
      input_devices[port] = RETRO_DEVICE_JOYPAD;
      break;

    case RETRO_DEVICE_MOUSE:
      input_devices[port] = RETRO_DEVICE_MOUSE;
      break;

    case RETRO_DEVICE_KEYBOARD:
      input_devices[port] = RETRO_DEVICE_KEYBOARD;
      break;

    default:
      if (log_cb) log_cb(RETRO_LOG_ERROR, "%s\n", "[libretro]: Invalid device, setting type to RETRO_DEVICE_JOYPAD ...");
      input_devices[port] = RETRO_DEVICE_JOYPAD;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_set_environment(retro_environment_t cb)
{
  environ_cb = cb;

  struct retro_variable variables[] = {
    // Adding more variables and rearranging them is safe.
    { "stella_console", "Console display; auto|ntsc|pal|secam|ntsc50|pal60|secam60"},
    { "stella_filter", "TV effects; disabled|composite|s-video|rgb|badly adjusted"},
    { "stella_ntsc_aspect", "NTSC aspect %; auto|86|87|88|89|90|91|92|93|94|95|96|97|98|99|100|101|102|103|104|105|106|107|108|109|110|111|112|113|114|115|116|117|118|119|120|121|122|123|124|125|50|75|76|77|78|79|80|81|82|83|84|85"},
    { "stella_pal_aspect", "PAL aspect %; auto|104|105|106|107|108|109|110|111|112|113|114|115|116|117|118|119|120|121|122|123|124|125|50|75|76|77|78|79|80|81|82|83|84|85|86|87|88|89|90|91|92|93|94|95|96|97|98|99|100|101|102|103"},
    { "stella_stereo", "Stereo sound; auto|off|on"},
    { "stella_palette", "Palette colors; standard|z26"},
    { "stella_phosphor", "Phosphor mode; auto|off|on"},
    { "stella_phosphor_blend", "Phosphor blend %; 60|65|70|75|80|85|90|95|100|0|5|10|15|20|25|30|35|40|45|50|55"},
    { NULL, NULL },
  };

  environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_init()
{
  struct retro_log_callback log;
  unsigned level = 4;

  log_cb = NULL;
  if(environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) log_cb = log.log;

  environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool retro_load_game(const struct retro_game_info *info)
{
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

  struct retro_input_descriptor desc[] = {
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "Fire" },

    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Left Difficulty A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     "Left Difficulty B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,     "Color" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "Right Difficulty A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "Right Difficulty B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     "Black/White" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Reset" },

    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "Fire" },

    { 0 },
  };

  if(!info || info->size >= stella.getROMMax()) return false;

  environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

  if(!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
  {
    if(log_cb) log_cb(RETRO_LOG_INFO, "[Stella]: XRGB8888 is not supported.\n");
    return false;
  }


  stella.setROM(info->data, info->size);

  return reset_system();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_reset() { stella.reset(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_run()
{
  bool updated = false;

  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
    update_variables();

  if(system_reset)
  {
    reset_system();
    update_system_av();
    return;
  }


  update_input();


  stella.runFrame();

  if(stella.getVideoResize())
    update_geometry();


  //printf("retro_run - %d %d %d - %d\n", stella.getVideoWidth(), stella.getVideoHeight(), stella.getVideoPitch(), stella.getAudioSize() );

  if(stella.getVideoReady())
    video_cb(stella.getVideoBuffer(), stella.getVideoWidth(), stella.getVideoHeight(), stella.getVideoPitch());

  if(stella.getAudioReady())
    audio_batch_cb(stella.getAudioBuffer(), stella.getAudioSize());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_unload_game() { stella.destroy(); }
void retro_deinit() { stella.destroy(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
size_t retro_serialize_size() { return stella.getStateMax(); }
bool retro_serialize(void *data, size_t size) { return stella.saveState(data, size); }
bool retro_unserialize(const void *data, size_t size) { return stella.loadState(data, size); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void *retro_get_memory_data(unsigned id)
{
  switch (id)
  {
    case RETRO_MEMORY_SYSTEM_RAM: return stella.getRAM();
    default: return NULL;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
size_t retro_get_memory_size(unsigned id)
{
  switch (id)
  {
    case RETRO_MEMORY_SYSTEM_RAM: return stella.getRAMSize();
    default: return 0;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void retro_cheat_reset() {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}
