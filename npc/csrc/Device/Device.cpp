#include <Device/Device.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <my_utils.h>
#include <print>
#include <stdexcept>

Devices::Devices(Devices::DeviceSettings ds, size_t MemSize,
                 std::string_view program, bool mtracer)
    : KeyboardBase{}, AudioBase{}, VideoBase{}, RTC{}, renderer(nullptr),
      texture(nullptr), window(nullptr), quit(false), device_settings(ds),
      mtracer(mtracer), operation({.used = false}), multiple_emu(false) {
  using std::ifstream;
  using std::ios;

  M.resize(MemSize / sizeof(uint32_t));
  std::filesystem::path program_path = program;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  if (!prog_file.good()) {
    log_and_throw<std::runtime_error>("Failed to open program file {}",
                                      program);
  }
  uint32_t size_prog = 0;
  prog_file.seekg(0, ios::end);
  size_prog = prog_file.tellg();
  prog_file.seekg(0, ios::beg);
  if (size_prog > MemSize * sizeof(uint32_t))
    log_and_throw<std::logic_error>(
        "Program size is bigger than memory size {}", MemSize);

  prog_file.read(reinterpret_cast<char *>(M.data()), size_prog);

  spdlog::info("Loaded {} words", size_prog);
  // cpu.pc = init_pc;
  prog_file.close();
}

constexpr std::array<uint32_t, 16> lookup_mask32 = {
    0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF, 0x00FF0000, 0x00FF00FF,
    0x00FFFF00, 0x00FFFFFF, 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
    0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF};

void Devices::writeMemory(uint32_t waddr, uint32_t wdata, uint32_t wmask) {
  uint32_t mask32 = lookup_mask32[wmask]; // extend 4bit mask to 32bit mask
#ifndef NO_DIFFTEST
  if (multiple_emu) [[unlikely]] {
    if (operation.used) {
      wdata &= mask32;
      if (operation.op !=
          OP::OP_record{
              .is_read = false, .addr = waddr, .wdata = wdata, .wmask = wmask})
        log_and_throw<std::logic_error>(
            "Different memory operation from ref\n"
            "dut : {:6} addr={:#010x} data={:#010x} mask={:x}\n"
            "ref : {:6} addr={:#10x} data={:#010x} mask={:x}",
            operation.op.is_read ? "read" : "write", operation.op.addr,
            operation.op.wdata, operation.op.wmask, "write", waddr, wdata,
            wmask);
      operation.used++;
      return;
    } else {
      operation.used++;
      operation.op = {.is_read = false,
                      .addr = waddr,
                      .wdata = wdata & mask32,
                      .wmask = wmask};
    }
  }
#endif
#ifndef DISABLE_ALL_TRACER
  if (mtracer) {
    println("Write to memory : {:#010x}, data : {:#010x}, mask : {:#010x}",
            (uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
  }
#endif

  uint32_t addr = (uint32_t)(waddr - Devices::memOffset) >> 2;
  if (addr < M.size() && waddr >= Devices::memOffset) [[likely]] {
    M[addr] &= ~mask32;
    M[addr] |= wdata & mask32;
  } else if (waddr >= Devices::deviceBase) {
    writeMMIO(waddr & ~0x3, mask32, wdata);
  }
}

uint32_t Devices::readMemory(uint32_t raddr) {
  raddr &= ~0x3;
#ifndef NO_DIFFTEST
  if (multiple_emu) [[unlikely]] {
    if (operation.used) {
      if (operation.op.is_read != true || operation.op.addr != raddr)
        log_and_throw<std::logic_error>(
            "Different memory operation from ref\n"
            "dut : {:6} addr={:#010x} data={:#010x} mask={:x}\n"
            "ref : {:6} addr={:#10x}",
            operation.op.is_read ? "read" : "write", operation.op.addr,
            operation.op.wdata, operation.op.wmask, "read", raddr);
      operation.used++;
      return operation.rdata;
    }
    operation.used++;
    operation.op.is_read = true;
    operation.op.addr = raddr;
  }
#endif

#ifndef DISABLE_ALL_TRACER
  if (mtracer) {
    println("Reading memory memory : {:#010x}", (uint32_t)raddr);
  }
#endif
  uint32_t rdata = 0xdeadbeef;

  uint32_t addr = (uint32_t)(raddr - Devices::PC_Init) >> 2;
  if (addr < M.size() && raddr >= Devices::PC_Init) [[likely]]
    rdata = M[addr];
  else if (raddr >= Devices::deviceBase)
    rdata = readMMIO(raddr).value_or(0xdeadbeef);
#ifndef NO_DIFFTEST
  operation.rdata = rdata;
#endif
  return rdata;
}

void Devices::init_ioe() {
  if (device_settings.enable_audio) {
    AudioBase.sbuf = std::make_unique_for_overwrite<uint8_t[]>(SoundBufferSize);
    AudioBase.reg_ctl.reg_sbuf_size = SoundBufferSize;
  }
  if (device_settings.enable_vga) {
    VideoBase.vmem1 = std::make_unique_for_overwrite<uint8_t[]>(VMemSize);
    VideoBase.vmem2 = std::make_unique_for_overwrite<uint8_t[]>(VMemSize);
    VideoBase.front_ptr = VideoBase.vmem1.get();
    VideoBase.back_ptr = VideoBase.vmem2.get();
    VideoBase.screen_size_info = (ScreenWidth << 16) | ScreenHeight;
    // init_vga();
  }
  if (device_settings.enable_keyboard) {
    key_queue = std::make_unique<lockfree::spsc::Queue<uint32_t, 1024>>();
  }
  device_alive = true;
  device_running = true;
  println("!!!!?");
  device_update_thread = std::thread(&Devices::device_update_loop, this);
}
void Devices::pause(bool is_paused) { device_running = !is_paused; }
void Devices::device_update_loop() {
  println("!!!");
  if (device_settings.enable_vga) {
    init_vga();
  }
  if (device_settings.enable_keyboard) {
    init_keyboard();
  }
  println("!!!!");
  using namespace std::chrono;
  auto last = steady_clock::now();
  try {
    while (device_alive) { // main device update loop, executed 60 times per
                           // second.
      if (device_running) {
        auto now = steady_clock::now();
        if (now - last < 16.667ms)
          continue;
        last = now;
        if (device_settings.enable_vga) {
          vga_update_screen();
        }
        if (device_settings.enable_keyboard) {
          process_keyboard();
        }
      }
    }
  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }
  if (texture)
    SDL_DestroyTexture(texture);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
}

void Devices::update_RTC() {
  using namespace std::chrono;
  auto now_tick = steady_clock().now();
  uint64_t duration = static_cast<uint64_t>(
      duration_cast<microseconds>(now_tick - RTC.last_time).count());
  // uint64_t start_time =
  //     RTC.RTC_reg[0] | (static_cast<uint64_t>(RTC.RTC_reg[1]) << 32);
  uint64_t now_time = duration;

  RTC.RTC_reg[0] = static_cast<uint32_t>(now_time & 0xFFFFFFFF);
  RTC.RTC_reg[1] = static_cast<uint32_t>(now_time >> 32);
  // RTC.last_time = now_tick;
  // std::print("!!{}\r", now_time);
}

Devices::~Devices() {
  device_running = false;
  device_alive = false;
  println("{}", device_update_thread.joinable());

  device_update_thread.join();
  println("!!");
  SDL_CloseAudio();
  SDL_Quit();
}