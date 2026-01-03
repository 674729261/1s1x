#include <Device/Device.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <my_utils.h>
#include <print>
#include <span>
#include <stdexcept>

static void write_mask(uint32_t &dst, uint32_t mask32, uint32_t wdata) {
  dst = (dst & ~mask32) | (wdata & mask32);
}

static void write_mask(std::atomic<uint32_t> &dst, uint32_t mask32,
                       uint32_t wdata) {
  uint32_t t = dst.load(), new_value;
  do {
    new_value = (t & ~mask32) | (wdata & mask32);
  } while (!dst.compare_exchange_weak(t, new_value));
}

static int check_addr_range(uint32_t addr, uint32_t base, uint32_t end) {
  if (addr < base || addr >= end)
    return -1;
  return (addr - base) >> 2;
}

std::optional<uint32_t> Devices::readMMIO(int raddr) {
  if (uint32_t RTC_id = check_addr_range(raddr, RTCAddr, RTCAddrEnd);
      RTC_id != -1) {
    update_RTC();
    if (raddr == RTCAddr)
      return RTC.RTC_reg[RTC_id];
    else
      return RTC.RTC_reg[RTC_id];
  }
  if (uint32_t VGA_FB_Offset =
          check_addr_range(raddr, VGAFBPort, VGAFBPort + VMemSize);
      device_settings.enable_vga && VGA_FB_Offset != -1) {
    return reinterpret_cast<uint32_t *>(VideoBase.back_ptr)[VGA_FB_Offset];
  }
  if (device_settings.enable_vga && raddr == VGAControlRegsPort) {
    return VideoBase.screen_size_info;
  }

  if (uint32_t AudioReg_id = check_addr_range(
          raddr, AudioPort, AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs);
      device_settings.enable_audio && AudioReg_id != -1) {

    // spdlog::info("Reading from AudioBase[{}]", AudioReg_id);

    std::span<uint32_t, AudioBase_t::n_regs> ctlreg_arrview(
        &AudioBase.reg_ctl.reg_freq, AudioBase_t::n_regs);
    return ctlreg_arrview[AudioReg_id];
  }

  if (uint32_t SoundBufferOffset = check_addr_range(
          raddr, SoundBufferPort, SoundBufferPort + SoundBufferSize);
      device_settings.enable_audio && SoundBufferOffset != -1) {
    return reinterpret_cast<uint32_t *>(
        AudioBase.sbuf.get())[SoundBufferOffset];
  }

  if (device_settings.enable_keyboard && raddr == KeyboardPort) {
    uint32_t key;
    if (key_queue->Pop(key)) {
      return key;
    }

    return 0;
  }
  return std::nullopt;
}

void Devices::writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata) {
  using namespace std::chrono;

  // if (waddr >= RTCAddr && waddr < RTCAddrEnd) {
  if (uint32_t RTC_id = check_addr_range(waddr, RTCAddr, RTCAddrEnd);
      RTC_id != -1) {
    update_RTC();
    write_mask(RTC.RTC_reg[RTC_id], mask32, wdata);
    RTC.last_time = steady_clock::now();
    return;
  }
  if (waddr == SerialPort) {
    if (mask32 != 0xFF)
      log_and_throw<std::logic_error>(
          "mask32 {:08x} is not 0x000000FF when writing serial port", mask32);
    std::cout.put(wdata);
    // std::cout.flush();
    return;
  }
  if (uint32_t VGA_FB_Offset =
          check_addr_range(waddr, VGAFBPort, VGAFBPort + VMemSize);
      VGA_FB_Offset != -1) {
    ensure_vga_enabled();
    write_mask(reinterpret_cast<uint32_t *>(VideoBase.back_ptr)[VGA_FB_Offset],
               mask32, wdata);
    return;
  }

  if (waddr == VGAControlRegsPort + sizeof(uint32_t)) {
    ensure_vga_enabled();
    uint32_t tmp = VideoBase.sync.load();
    if (!tmp) {
      write_mask(tmp, mask32, wdata);
      if (tmp) {
        VideoBase.back_ptr = VideoBase.front_ptr.exchange(VideoBase.back_ptr);
        uint8_t *f = VideoBase.front_ptr;
        std::copy(f, f + Devices::VMemSize, VideoBase.back_ptr);
      }
      VideoBase.sync.store(tmp);
    }
    return;
  }

  // if (waddr >= AudioPort &&
  //     waddr < AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs) {
  if (uint32_t AudioReg_id = check_addr_range(
          waddr, AudioPort, AudioPort + sizeof(uint32_t) * AudioBase_t::n_regs);
      AudioReg_id != -1) {
    ensure_audio_enabled();
    // spdlog::info("Writing to AudioBase[{}]", AudioReg_id);

    std::span<uint32_t, AudioBase_t::n_regs> ctlreg_arrview(
        &AudioBase.reg_ctl.reg_freq, AudioBase_t::n_regs);

    write_mask(ctlreg_arrview[AudioReg_id], mask32, wdata);
    if (AudioBase.reg_ctl.reg_init) {
      init_audio();
      AudioBase.reg_ctl.reg_init = 0;
    }

    return;
  }

  if (uint32_t SoundBufferOffset = check_addr_range(
          waddr, SoundBufferPort, SoundBufferPort + SoundBufferSize);
      SoundBufferOffset != -1) {
    ensure_audio_enabled();
    SDL_LockAudio();
    write_mask(
        reinterpret_cast<uint32_t *>(AudioBase.sbuf.get())[SoundBufferOffset],
        mask32, wdata);
    SDL_UnlockAudio();

    return;
  }

  log_and_throw<std::logic_error>("Writing to invalid MMIO {:08x}", waddr);
}