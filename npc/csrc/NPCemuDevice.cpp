#include "Simulators/NPCemu.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>
static NPCemu::AudioBase_t *curAudioBase;

void NPCemu::init_ioe() {
  if (device_settings.enable_audio) {
    AudioBase.sbuf = std::make_unique<uint8_t[]>(SoundBufferSize);
    AudioBase.reg_sbuf_size = SoundBufferSize;
  }
  if (device_settings.enable_vga) {
    init_vga();
  }
  if (device_settings.enable_keyboard) {
    init_keyboard();
  }
}

void NPCemu::ensure_audio_enabled() {
  if (!device_settings.enable_audio) {
    spdlog::error("Accessing audio MMIO when audio is disabled");
    throw std::logic_error("Accessing audio MMIO when audio is disabled");
  }
}

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
    spdlog::error("Can't open audio - %s\n", SDL_GetError());
    throw std::runtime_error(
        std::format("Can't open audio - %s\n", SDL_GetError()));
  }
  AudioBase.reg_init = 0;
  SDL_PauseAudio(0);
}

void NPCemu::init_keyboard() {}
void NPCemu::init_vga() {}

static void write_mask(uint32_t &dst, uint32_t mask32, uint32_t wdata) {
  dst &= ~mask32;
  dst |= wdata & mask32;
}

static int check_addr_range(uint32_t addr, uint32_t base, uint32_t end) {
  if (addr < base || addr >= end)
    return -1;
  return (addr - base) >> 2;
}

std::optional<uint32_t> NPCemu::readMMIO(int raddr) {
  if (uint32_t RTC_id = check_addr_range(raddr, RTCAddr, RTCAddrEnd);
      RTC_id != -1) {
    update_RTC();
    if (raddr == RTCAddr)
      return RTC.RTC_reg[RTC_id];
    else
      return RTC.RTC_reg[RTC_id];
  }

  if (uint32_t AudioReg_id = check_addr_range(
          raddr, AudioPort, AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs);
      AudioReg_id != -1) {
    ensure_audio_enabled();
    return reinterpret_cast<uint32_t *>(&AudioBase)[AudioReg_id];
  }

  if (uint32_t SoundBufferOffset = check_addr_range(
          raddr, SoundBufferPort, SoundBufferPort + SoundBufferSize);
      SoundBufferOffset != -1) {
    ensure_audio_enabled();
    return reinterpret_cast<uint32_t *>(
        AudioBase.sbuf.get())[SoundBufferOffset];
  }

  return std::nullopt;
}

void NPCemu::writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata) {
  using namespace std::chrono;
  // if (waddr >= RTCAddr && waddr < RTCAddrEnd) {
  if (uint32_t RTC_id = check_addr_range(waddr, RTCAddr, RTCAddrEnd);
      RTC_id != -1) {
    update_RTC();
    write_mask(RTC.RTC_reg[RTC_id], mask32, wdata);
    RTC.last_time = steady_clock().now();
    return;
  }
  if (waddr == SerialPort) {
    if (mask32 != 0xFF)
      throw std::logic_error(std::format(
          "mask32 {:08x} is not 0xFF when writing serial port", mask32));
    std::cout.put(wdata);
    // std::cout.flush();
    return;
  }

  // if (waddr >= AudioPort &&
  //     waddr < AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs) {
  if (uint32_t AudioReg_id = check_addr_range(
          waddr, AudioPort, AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs);
      AudioReg_id != -1) {
    ensure_audio_enabled();
    write_mask(reinterpret_cast<uint32_t *>(&AudioBase)[AudioReg_id], mask32,
               wdata);
    if (AudioBase.reg_init)
      init_audio();
    return;
  }

  if (uint32_t SoundBufferOffset = check_addr_range(
          waddr, SoundBufferPort, SoundBufferPort + SoundBufferSize);
      SoundBufferOffset != -1) {
    ensure_audio_enabled();
    write_mask(
        reinterpret_cast<uint32_t *>(AudioBase.sbuf.get())[SoundBufferOffset],
        mask32, wdata);

    return;
  }

  throw std::logic_error(std::format("Writing to invalid MMIO {:08x}", waddr));
}