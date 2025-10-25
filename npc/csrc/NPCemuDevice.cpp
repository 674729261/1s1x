#include "Simulators/NPCemu.h"
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
static NPCemu::AudioBase_t *curAudioBase;
static void fill_audio_callback(void *udata, Uint8 *stream, int len) {
  SDL_memset(stream, 0, len);
  static int last_pos = 0;
  if (len == 0) {
    return;
  }
  if (len > curAudioBase->reg_count)
    len = curAudioBase->reg_count;
  if (last_pos + len <= NPCemu::SoundBufferSize) {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos, len,
                 SDL_MIX_MAXVOLUME);
    last_pos += len;
  } else {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos,
                 NPCemu::SoundBufferSize - last_pos, SDL_MIX_MAXVOLUME);
    SDL_MixAudio(stream + NPCemu::SoundBufferSize - last_pos,
                 static_cast<uint8_t *>(udata),
                 len - NPCemu::SoundBufferSize + last_pos, SDL_MIX_MAXVOLUME);
    last_pos = len - NPCemu::SoundBufferSize + last_pos;
  }

  curAudioBase->reg_count -= len;
}
void NPCemu::init_audio() {

  if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    spdlog::error("Could not initialize SDL - {}\n", SDL_GetError());
    throw std::runtime_error(
        std::format("Could not initialize SDL - {}\n", SDL_GetError()));
  }
  SDL_CloseAudio();
  SDL_AudioSpec sdlAudioSpec = {
      .freq = static_cast<int>(AudioBase.reg_freq),
      .format = AUDIO_S16SYS,
      .channels = static_cast<Uint8>(AudioBase.reg_channels),
      .silence = 0,
      .samples = static_cast<Uint16>(AudioBase.reg_samples),
      .callback = fill_audio_callback,
      .userdata = AudioBase.sbuf.get()};
  if (SDL_OpenAudio(&sdlAudioSpec, NULL) < 0) {
    fprintf(stderr, "Can't open audio - %s\n", SDL_GetError());
    exit(-1);
  }
  AudioBase.reg_init = 0;
  SDL_PauseAudio(0);
}

void NPCemu::init_keyboard() {}
void NPCemu::init_vga() {}