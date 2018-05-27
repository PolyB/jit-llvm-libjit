#include "lj/Types.hh"

namespace lj
{

jit_type_t Types::get_bfstate()
{
  static jit_type_t val;
  if (val)
    return val;
  jit_type_t v[2] = {
        jit_type_create_pointer(jit_type_ubyte, true),
        jit_type_ubyte
      };
  val = jit_type_create_struct(
      v,
      2,
      true);
  return val;
}

jit_type_t Types::get_bfstate_ptr()
{
  static jit_type_t val = jit_type_create_pointer(Types::get_bfstate(), true);
  return val;
}

jit_type_t Types::get_fp_bfstate()
{
  jit_type_t types[1] = {Types::get_bfstate_ptr()};
  static jit_type_t val =
    jit_type_create_signature(
        jit_abi_cdecl,
        jit_type_void,
        types, 1, true);
  return val;
}

} // namespace lj
