#pragma once

struct JitAddrU{
  virtual ~JitAddrU() = default;
  JitAddrU(const JitAddrU&) = delete;
  JitAddrU(JitAddrU&&) = delete;
  JitAddrU& operator=(JitAddrU&&) = delete;
  JitAddrU& operator=(const JitAddrU&) = delete;
  protected:
    JitAddrU() = default;
};
