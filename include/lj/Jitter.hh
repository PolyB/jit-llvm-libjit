#pragma once

#include <jit/jit.h>

#include "JitterBase.hh"
#include "JitFun.hh"

namespace lj
{

struct Jitter : public ::JitterBase
{
  public:
    Jitter();
    ~Jitter() override;

    Jitter(const Jitter&) = delete;
    Jitter(Jitter&&) = delete;
    Jitter& operator=(Jitter&&) = delete;
    Jitter& operator=(const Jitter&) = delete;

    std::unique_ptr<JitFunBase> make_fun() override;
  private:
    jit_context_t context_;
};

} // namespace lj
