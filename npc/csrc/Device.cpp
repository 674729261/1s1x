#include "Device/Audio.h"
#include "Device/Keyboard.h"
#include "Device/VGA.h"
#include "spdlog/spdlog.h"
#include <Device/Device.h>
#include <SDL2/SDL_events.h>
#include <Simulate.h>

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Window *window = NULL;

static void fill_audio_callback(void *udata, Uint8 *stream, int len) {
  SDL_memset(stream, 0, len);
  static int last_pos = 0;
  if (len == 0) {
    return;
  }
  uint32_t cnt = Audio.curAudioBase->reg_ctl.reg_count;
  if (len > cnt)
    len = cnt;
  if (last_pos + len <= AudioBase_t::SoundBufferSize) {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos, len,
                 SDL_MIX_MAXVOLUME);
    last_pos += len;
  } else {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos,
                 AudioBase_t::SoundBufferSize - last_pos, SDL_MIX_MAXVOLUME);
    SDL_MixAudio(stream + AudioBase_t::SoundBufferSize - last_pos,
                 static_cast<uint8_t *>(udata),
                 len - AudioBase_t::SoundBufferSize + last_pos,
                 SDL_MIX_MAXVOLUME);
    last_pos = len - AudioBase_t::SoundBufferSize + last_pos;
  }

  Audio.curAudioBase->reg_ctl.reg_count -= len;
}

static void VGA_init() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(VideoBase_t::ScreenWidth * 2,
                              VideoBase_t::ScreenHeight * 2, 0, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "RISCV32E-NPC");
  texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
      VideoBase_t::ScreenWidth, VideoBase_t::ScreenHeight);
  SDL_RenderPresent(renderer);
}

static void keyboard_init() {}
static void audio_init() {
  if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    log_and_throw<std::runtime_error>("Could not initialize SDL - {}\n",
                                      SDL_GetError());
  }
  SDL_CloseAudio();
  if (Audio.reg_ctl.reg_samples >= (1 << 16)) {
    log_and_throw<std::logic_error>(
        "Audio.reg_samples = {} is bigger than 65535",
        Audio.reg_ctl.reg_samples);
  }
  if (Audio.reg_ctl.reg_channels >= (1 << 8)) {
    log_and_throw<std::logic_error>(
        "Audio.reg_channels = {} is bigger than 255",
        Audio.reg_ctl.reg_channels);
  }
  SDL_AudioSpec sdlAudioSpec = {
      .freq = static_cast<int>(Audio.reg_ctl.reg_freq),
      .format = AUDIO_S16SYS,
      .channels = static_cast<Uint8>(Audio.reg_ctl.reg_channels),
      .silence = 0,
      .samples = static_cast<Uint16>(Audio.reg_ctl.reg_samples),
      .callback = fill_audio_callback,
      .userdata = Audio.sbuf.get()};
  if (SDL_OpenAudio(&sdlAudioSpec, NULL) < 0) {
    log_and_throw<std::runtime_error>("Can't open audio - %s\n",
                                      SDL_GetError());
  }
  Audio.curAudioBase = &Audio;
  SDL_PauseAudio(0);
}

static void keyboard_update() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      {
        uint8_t k = event.key.keysym.scancode;
        bool is_keydown = (event.key.type == SDL_KEYDOWN);
        uint32_t wrapped = wrap_key_event(k, is_keydown);

        if (!Keyboard.kbd_buf->Push(wrapped))
          spdlog::warn("Key event {:#08x} ignored because the buffer is full",
                       wrapped);
      }
    } else if (event.type == SDL_QUIT) {
      sim_state.store(SimulationState::QUIT);
    }
  }
}

static void vga_update() {
  uint8_t *ptr_vmem = reinterpret_cast<uint8_t *>(Video.front_ptr.load());
  SDL_UpdateTexture(texture, NULL, ptr_vmem,
                    VideoBase_t::ScreenWidth * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void device_thread_work(std::promise<void> &device_inited_promise) {
  VGA_init();
  keyboard_init();

  device_inited_promise.set_value();

  using namespace std::chrono_literals;
  auto last_tick_vga = std::chrono::steady_clock::now();
  auto last_tick_kbd = std::chrono::steady_clock::now();
  while (!quit.load()) {
    auto cur_tick = std::chrono::steady_clock::now();
    if (Video.sync.load() && cur_tick - last_tick_vga >= 16.667ms) {
      last_tick_vga = cur_tick;
      vga_update();
      Video.sync.store(0);
    }

    if (cur_tick - last_tick_kbd >= 16.667ms) {
      last_tick_kbd = cur_tick;
      keyboard_update();
    }
  }
  if (texture)
    SDL_DestroyTexture(texture);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
}