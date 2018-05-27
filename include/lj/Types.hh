#pragma once

#include <jit/jit.h>

// libjit types are hard to initialize, so this type hold their singleton

namespace lj
{

class Types
{
  public:
  Types() = delete;

  static jit_type_t get_bfstate(); // BFState
  static jit_type_t get_bfstate_ptr(); // BFState*
  static jit_type_t get_fp_bfstate(); // void (*)(BFState*)
};

} // namespace lj
