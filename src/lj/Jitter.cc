#include "lj/Jitter.hh"

namespace lj
{

Jitter::Jitter()
  :context_(jit_context_create())
{}

Jitter::~Jitter()
{
  jit_context_destroy(context_);
}

std::unique_ptr<JitFunBase> Jitter::make_fun()
{
  return std::make_unique<JitFun>(context_);
}

} // namespace lj
