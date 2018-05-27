#pragma once

struct JitAddr{
  virtual ~JitAddr() = default;
  JitAddr(const JitAddr&) = delete;
  JitAddr(JitAddr&&) = delete;
  JitAddr& operator=(JitAddr&&) = delete;
  JitAddr& operator=(const JitAddr&) = delete;
  protected:
    JitAddr() = default;
};
