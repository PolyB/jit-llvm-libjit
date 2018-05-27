#pragma once

#include <cstdint>
#include <jit/jit.h>

namespace lj
{
  namespace Utils
  {
    template <class>
    class Type;
    template <> struct Type<uint8_t> { static constexpr jit_type_t type = jit_type_ubyte; };
    template <> struct Type<int8_t> { static constexpr jit_type_t type = jit_type_byte; };
    template <> struct Type<uint16_t> { static constexpr jit_type_t type = jit_type_ushort; };
    template <> struct Type<int16_t> { static constexpr jit_type_t type = jit_type_short; };
    template <> struct Type<uint32_t> { static constexpr jit_type_t type = jit_type_uint; };
    template <> struct Type<int32_t> { static constexpr jit_type_t type = jit_type_int; };
    template <> struct Type<uint64_t> { static constexpr jit_type_t type = jit_type_ulong; };
    template <> struct Type<int64_t> { static constexpr jit_type_t type = jit_type_long; };
    template <> struct Type<float> { static constexpr jit_type_t type = jit_type_ufloat32; };
    template <> struct Type<float> { static constexpr jit_type_t type = jit_type_float32 ; };
    template <> struct Type<double> { static constexpr jit_type_t type = jit_type_ufloat64; };
    template <> struct Type<double> { static constexpr jit_type_t type = jit_type_float64; };
    template <> struct Type<void *> { static constexpr jit_type_t type = jit_type_void_ptr; };

  } // namespace Utils
} // namespace lj
