#include "jit/jit-dump.h"
#include "lj/JitFun.hh"
#include "lj/Types.hh"
#include "configs.hh"

namespace lj
{

JitFun::JitFun(jit_context_t context)
  :context_(context)
{
  jit_context_build_start(context_);
  fun_ = jit_function_create(context_, Types::get_fp_bfstate());
  bf_state_ = jit_value_get_param(fun_, 0);

  pos_ = jit_insn_load_relative(
      fun_,
      bf_state_, // value
      offsetof(BFState, pos), // offset
      jit_type_uint);
  mem_ = jit_insn_load_relative(
      fun_,
      bf_state_, // value
      offsetof(BFState, mem), // offset
      jit_type_void_ptr);
}

JitFun::~JitFun()
{
  jit_context_build_end(context_);
}


void JitFun::emit_add()
{
  jit_value_t j_one = jit_value_create_nint_constant(fun_, jit_type_uint, 1);

  jit_value_t v = jit_insn_load_elem(
      fun_,
      mem_,
      pos_,
      jit_type_uint);

  v = jit_insn_add(
    fun_,
    v,
    j_one);

  jit_insn_store_elem(
      fun_,
      mem_,
      pos_,
      v);
}

void JitFun::emit_sub()
{
  jit_value_t j_one = jit_value_create_nint_constant(fun_, jit_type_uint, 1);

  jit_value_t v = jit_insn_load_elem(
      fun_,
      mem_,
      pos_,
      jit_type_uint);

  v = jit_insn_sub(
    fun_,
    v,
    j_one);

  jit_insn_store_elem(
      fun_,
      mem_,
      pos_,
      v);
}

void JitFun::emit_left()
{
  jit_value_t j_one = jit_value_create_nint_constant(fun_, jit_type_uint, 1);

  jit_label_t lb = jit_label_undefined;

  jit_insn_branch_if_not(
      fun_,
      pos_,
      &lb);

  jit_value_t v = jit_insn_sub(
      fun_,
      pos_,
      j_one); // constant 1
  jit_insn_store(fun_, pos_, v);

  jit_insn_label(fun_, &lb);
}

void JitFun::emit_right()
{
  jit_value_t j_one = jit_value_create_nint_constant(fun_, jit_type_uint, 1);
  jit_value_t j_maxpos = jit_value_create_nint_constant(fun_, jit_type_uint, configs::vector_size);

  jit_label_t lb = jit_label_undefined;

  jit_value_t test = jit_insn_eq(fun_, pos_, j_maxpos);

  jit_insn_branch_if(
      fun_,
      test,
      &lb);

  jit_value_t v = jit_insn_add(
      fun_,
      pos_,
      j_one); // constant 1
  jit_insn_store(fun_, pos_, v);

  jit_insn_label(fun_, &lb);
}

void JitFun::emit_call(const char *str, void (*fptr)(BFState*))
{
  jit_insn_store_relative(
      fun_,
      bf_state_,
      offsetof(BFState, pos), // offset
      pos_);

  jit_insn_call_native(
      fun_,
      str,
      reinterpret_cast<void*>(fptr),
      Types::get_fp_bfstate(),
      &bf_state_,
      1,
      0);

  pos_ = jit_insn_load_relative(
      fun_,
      bf_state_, // value
      offsetof(BFState, pos), // offset
      jit_type_uint);

}

std::unique_ptr<JitAddr> JitFun::emit_add_label()
{
  auto l = std::make_unique<Addr>(jit_label_undefined);
  jit_insn_label(fun_, &l->val);
  return l;
}

std::unique_ptr<JitAddrU> JitFun::emit_future_label()
{
  return std::make_unique<Addr>(jit_label_undefined);
}

void JitFun::set_future_label(JitAddrU* addr)
{
  if (Addr* a = dynamic_cast<Addr*>(addr))
    jit_insn_label(fun_, &a->val);
  else
    throw std::runtime_error("libjit: cannot set_future_label: bad JitAddrU");

}

void JitFun::emit_jz(JitAddr* addr)
{
  if (Addr* a = dynamic_cast<Addr*>(addr))
  {
    jit_value_t v = jit_insn_load_elem(
        fun_,
        mem_,
        pos_,
        jit_type_uint);

    jit_insn_branch_if_not(
        fun_,
        v,
        &a->val);
  }
  else
    throw std::runtime_error("libjit: cannot emit jz: bad JitAddr");
}

void JitFun::emit_jz(JitAddrU* addr)
{
  if (Addr* a = dynamic_cast<Addr*>(addr))
  {
    jit_value_t v = jit_insn_load_elem(
        fun_,
        mem_,
        pos_,
        jit_type_uint);

    jit_insn_branch_if_not(
        fun_,
        v,
        &a->val);
  }
  else
    throw std::runtime_error("libjit: cannot emit jz: bad JitAddrU");
}

void JitFun::emit_jnz(JitAddr* addr)
{
  if (Addr* a = dynamic_cast<Addr*>(addr))
  {
    jit_value_t v = jit_insn_load_elem(
        fun_,
        mem_,
        pos_,
        jit_type_uint);

    jit_insn_branch_if(
        fun_,
        v,
        &a->val);
  }
  else
    throw std::runtime_error("libjit: cannot emit jz: bad JitAddr");
}

void JitFun::emit_jnz(JitAddrU* addr)
{
  if (Addr* a = dynamic_cast<Addr*>(addr))
  {
    jit_value_t v = jit_insn_load_elem(
        fun_,
        mem_,
        pos_,
        jit_type_uint);

    jit_insn_branch_if(
        fun_,
        v,
        &a->val);
  }
  else
    throw std::runtime_error("libjit: cannot emit jz: bad JitAddrU");
}

std::unique_ptr<JittedFun> JitFun::gen_fun()
{
  jit_insn_store_relative(
      fun_,
      bf_state_,
      offsetof(BFState, pos), // offset
      pos_);
  jit_insn_store_relative(
      fun_,
      bf_state_,
      offsetof(BFState, mem), // offset
      mem_);

  jit_function_compile(fun_);
  jit_context_build_end(context_);
  auto jittedfun = std::make_unique<JFun>(reinterpret_cast<void(*)(BFState*)>(jit_function_to_closure(fun_)));

  return jittedfun;
}


void JitFun::dump()
{
  jit_dump_function(stderr, fun_, "fun");
}

void JitFun::emit_ret()
{

}
} // namespace lj
