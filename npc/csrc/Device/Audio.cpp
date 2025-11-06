#include <Device/Device.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <Simulators/NPCemu.h>
#include <cstdint>
#include <format>
#include <my_utils.h>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>
static AudioBase_t *curAudioBase;

static void fill_audio_callback(void *udata, Uint8 *stream, int len) {
  SDL_memset(stream, 0, len);
  static int last_pos = 0;
  if (len == 0) {
    return;
  }
  uint32_t cnt = curAudioBase->reg_ctl.reg_count;
  if (len > cnt)
    len = cnt;
  if (last_pos + len <= Devices::SoundBufferSize) {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos, len,
                 SDL_MIX_MAXVOLUME);
    last_pos += len;
  } else {
    SDL_MixAudio(stream, static_cast<uint8_t *>(udata) + last_pos,
                 Devices::SoundBufferSize - last_pos, SDL_MIX_MAXVOLUME);
    SDL_MixAudio(stream + Devices::SoundBufferSize - last_pos,
                 static_cast<uint8_t *>(udata),
                 len - Devices::SoundBufferSize + last_pos, SDL_MIX_MAXVOLUME);
    last_pos = len - Devices::SoundBufferSize + last_pos;
  }

  curAudioBase->reg_ctl.reg_count -= len;
}
void Devices::init_audio() {

  if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    log_and_throw<std::runtime_error>("Could not initialize SDL - {}\n",
                                      SDL_GetError());
  }
  SDL_CloseAudio();
  if (AudioBase.reg_ctl.reg_samples >= (1 << 16)) {
    log_and_throw<std::logic_error>(
        "AudioBase.reg_samples = {} is bigger than 65535",
        AudioBase.reg_ctl.reg_samples);
  }
  if (AudioBase.reg_ctl.reg_channels >= (1 << 8)) {
    log_and_throw<std::logic_error>(
        "AudioBase.reg_channels = {} is bigger than 255",
        AudioBase.reg_ctl.reg_channels);
  }
  SDL_AudioSpec sdlAudioSpec = {
      .freq = static_cast<int>(AudioBase.reg_ctl.reg_freq),
      .format = AUDIO_S16SYS,
      .channels = static_cast<Uint8>(AudioBase.reg_ctl.reg_channels),
      .silence = 0,
      .samples = static_cast<Uint16>(AudioBase.reg_ctl.reg_samples),
      .callback = fill_audio_callback,
      .userdata = AudioBase.sbuf.get()};
  if (SDL_OpenAudio(&sdlAudioSpec, NULL) < 0) {
    log_and_throw<std::runtime_error>("Can't open audio - %s\n",
                                      SDL_GetError());
  }
  curAudioBase = &AudioBase;
  SDL_PauseAudio(0);
}

void Devices::ensure_audio_enabled() {
  if (!device_settings.enable_audio) {
    log_and_throw<std::logic_error>(
        "Accessing audio MMIO when audio is disabled");
  }
}