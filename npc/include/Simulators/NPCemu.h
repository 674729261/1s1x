#pragma once

#include "RISCV32.h"
#include "my_utils.h"
#include "verilated.h"
#include <Device/Audio.h>
#include <Device/Device.h>
#include <Device/Keyboard.h>
#include <Device/VGA.h>
#include <VCPU.h>
#include <cstdint>
#include <functional>
#include <lockfree/spsc/queue.hpp>
#include <memory>
#include <print>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

extern "C" void trap(int signal);
extern "C" int pmem_read(int raddr, int clk, int valid);
extern "C" void pmem_write(int waddr, int wdata, char wmask);

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

class ProgSymTab;

class FetchProxy {
public:
  FetchProxy(std::shared_ptr<Devices> devices = nullptr)
      : current_time(0), sub_id(0), gen(1234), dist(1, 20) {}
  void update_one_cycle() {
    current_time++;
    while (!event_pool.empty() && event_pool.top().event_time <= current_time) {
      event_pool.top().func();
      event_pool.pop();
    }
  }

  void fetch_inst(Devices &devices, TOP_NAME &dut) {
    if (dut.io_inst_bus_reqValid) {
      struct {
        uint32_t pc;
        Devices &devices;
        decltype(dut.io_inst_bus_instr) &bus_instr;
        decltype(dut.io_inst_bus_respValid) &resp;
      } pack = {.pc = dut.io_inst_bus_ifu_addr,
                .devices = devices,
                .bus_instr = dut.io_inst_bus_instr,
                .resp = dut.io_inst_bus_respValid};

      int delay = dist(gen);
      register_event(
          [pack] {
            println("addr : {:08x}", pack.pc);
            pack.bus_instr = pack.devices.get_instruction(pack.pc);
            pack.resp = 1;
            println("inst : {:08x}", pack.bus_instr);
          },
          delay);
      register_event([&resp = dut.io_inst_bus_respValid] { resp = 0; },
                     delay + 1);
    }
  }

  void fetch_ram(Devices &devices, TOP_NAME &dut) {
    if (dut.io_mem_reqValid) {
      struct {
        uint32_t raddr;
        uint32_t waddr;
        uint32_t wdata;
        uint32_t wmask;
        Devices &devices = devices;
        decltype(dut.io_mem_rdata) rdata;
        decltype(dut.io_mem_respValid) resp;
      } pack = {.waddr = dut.io_inst_bus_ifu_addr,
                .wdata = dut.io_mem_wdata,
                .wmask = dut.io_mem_wmask,
                .devices = devices,
                .rdata = dut.io_mem_rdata,
                .resp = dut.io_mem_respValid};
      int delay = dist(gen);
      if (dut.io_mem_wen) {
        register_event(
            [pack] {
              pack.devices.writeMemory(pack.waddr, pack.wdata, pack.wmask);
              pack.resp = 1;
            },
            delay);
        register_event([&resp = dut.io_mem_respValid] { resp = 0; }, delay + 1);
      } else {
        register_event(
            [pack] {
              pack.rdata = pack.devices.readMemory(pack.raddr);
              pack.resp = 1;
            },
            delay);
        register_event([&resp = dut.io_mem_respValid] { resp = 0; }, delay + 1);
      }
    }
  }

  void clear() {
    while (!event_pool.empty())
      event_pool.pop();
  }

private:
  std::mt19937_64 gen;
  std::uniform_int_distribution<long long> dist;
  void register_event(std::function<void()> call, long long delay) {
    if (delay < 0)
      log_and_throw<std::logic_error>("Delay {} must be > 0", delay);
    else {
      event_pool.push({current_time + delay, sub_id, std::move(call)});
      sub_id++;
    }
  }

private:
  struct Request {
    long long event_time, sub_id;
    std::function<void()> func;
  };

  struct CompareRequest {
    bool operator()(const Request &lhs, const Request &rhs) const {
      if (lhs.event_time == rhs.event_time)
        return lhs.sub_id > rhs.sub_id;
      return lhs.event_time > rhs.event_time;
    }
  };

  std::priority_queue<Request, std::vector<Request>, CompareRequest> event_pool;
  long long current_time;
  long long sub_id;
};

class NPCemu : public RISCV32 {
public:
  NPCemu();

  addr_t getPC() override final;

  void reset() override final;
  void step() override final;
  unsigned long long instrCount() override final;

  // void writeMemory(int waddr, int wdata, char wmask) override final;
  // uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

  // friend void trap(int signal);
  // friend int pmem_read(int raddr);
  // friend void pmem_write(int waddr, int wdata, char wmask);

  ~NPCemu();

private:
  VerilatedContext context;
  TOP_NAME dut;
  int trapped;
  unsigned long long inst_count;
  FetchProxy proxy;

private:
  // void record_ftracer(uint32_t cur_inst, std::shared_ptr<ProgSymTab> sy_tab);
};
