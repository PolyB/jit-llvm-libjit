#pragma once

#include <memory>

#include "BFState.hh"
#include "JitAddr.hh"
#include "JitAddrU.hh"
#include "JittedFun.hh"


class JitFunBase
{
  public:
    JitFunBase() = default;
    JitFunBase(const JitFunBase&) = delete;
    JitFunBase(JitFunBase&&) = delete;
    JitFunBase& operator=(JitFunBase&&) = delete;
    JitFunBase& operator=(const JitFunBase&) = delete;
    virtual ~JitFunBase() = default;

    virtual void emit_add() = 0;
    virtual void emit_sub() = 0;
    virtual void emit_left() = 0;
    virtual void emit_right() = 0;
    virtual void emit_call(const char *name, void (*fptr)(BFState*)) = 0; // assert that fptr do not change mem
    virtual std::unique_ptr<JitAddr> emit_add_label() = 0;
    virtual std::unique_ptr<JitAddrU> emit_future_label() = 0;
    virtual void set_future_label(JitAddrU*) = 0;
    virtual void emit_jz(JitAddr*) = 0;
    virtual void emit_jz(JitAddrU*) = 0;
    virtual void emit_jnz(JitAddr*) = 0;
    virtual void emit_jnz(JitAddrU*) = 0;
    virtual std::unique_ptr<JittedFun> gen_fun() = 0;
    virtual void dump() = 0;
    virtual void emit_ret() = 0;
};
