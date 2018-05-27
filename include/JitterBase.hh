#pragma once

#include <memory>

#include "BFState.hh"
#include "JitFunBase.hh"
#include "JittedFun.hh"


class JitterBase
{
  public:
    JitterBase() = default;
    virtual ~JitterBase() = default;
    JitterBase(const JitterBase&) = delete;
    JitterBase(JitterBase&&) = delete;
    JitterBase& operator=(JitterBase&&) = delete;
    JitterBase& operator=(const JitterBase&) = delete;


    virtual std::unique_ptr<JitFunBase> make_fun() = 0;
};
