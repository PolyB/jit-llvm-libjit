#pragma once

#include "JitFunBase.hh"
#include "JitAddr.hh"

#include <jit/jit.h>

namespace lj
{

class JitFun : public ::JitFunBase
{
  public:
    explicit JitFun(jit_context_t context);
    JitFun(const JitFun&) = delete;
    JitFun(JitFun&&) = delete;
    JitFun& operator=(JitFun&&) = delete;
    JitFun& operator=(const JitFun&) = delete;
    ~JitFun() override;

    void emit_add() override;
    void emit_sub() override;
    void emit_left() override;
    void emit_right() override;
    void emit_call(const char *name, void (*fptr)(BFState*)) override;
    std::unique_ptr<JitAddr> emit_add_label() override;
    std::unique_ptr<JitAddrU> emit_future_label() override;
    void set_future_label(JitAddrU*) override;
    void emit_jz(JitAddr*) override;
    void emit_jz(JitAddrU*) override;
    void emit_jnz(JitAddr*) override;
    void emit_jnz(JitAddrU*) override;
    void dump() override;
    void emit_ret() override;

    std::unique_ptr<JittedFun> gen_fun() override;
  protected:
    struct Addr : public JitAddr, public JitAddrU
    {
      explicit Addr(jit_label_t val):val(val){}
      ~Addr() override = default;
      jit_label_t val;
    };

    struct JFun : public JittedFun
    {
      explicit JFun(void (*f)(BFState *)) :f(f)
      {}
      void call(BFState *s) override { f(s); }
      void (*f)(BFState *);
    };

    jit_context_t context_;
    jit_function_t fun_;
    jit_value_t bf_state_;

    jit_value_t pos_;
    jit_value_t mem_;
};

} // namespace lj
