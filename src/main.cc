#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <vector>

#include "JitterBase.hh"
#include "lj/Jitter.hh"
#include "ll/Jitter.hh"

[[noreturn]] void help(const char *pname)
{
  std::cout << "usage: " << pname << " --llvm in.bf" << std::endl
            << "       " << pname << " --libjit in.bf" << std::endl;
  exit(1);
}

std::unique_ptr<JitterBase> get_jitter(int argc, const char **argv)
{
  if (argc != 3)
    help(argv[0]);
  else if (argv[1] == std::string("--libjit"))
    return std::make_unique<lj::Jitter>();
  else if (argv[1] == std::string("--llvm"))
    return std::make_unique<ll::Jitter>();
  else
    help(argv[0]);
}

std::string get_program(std::ifstream& s)
{
  // use stream iterators to copy the stream to a string
  return std::string(std::istream_iterator<char>(s),
                     std::istream_iterator<char>());

}

using BranchDatas = std::pair<std::unique_ptr<JitAddr>, std::unique_ptr<JitAddrU>>;

std::unique_ptr<JittedFun> do_jit(JitterBase * jit, const std::string& program)
{
  std::stack<BranchDatas, std::vector<BranchDatas>> branches;
  auto fun = jit->make_fun();
  for (char c : program)
  {
    switch (c)
    {
      case '>':
        fun->emit_right();
        break;
      case '<':
        fun->emit_left();
        break;
      case '+':
        fun->emit_add();
        break;
      case '-':
        fun->emit_sub();
        break;
      case '.':
        fun->emit_call("do_get", +[](BFState *s){ std::cout << static_cast<char>(s->mem[s->pos]); });
        break;
      case ',':
        fun->emit_call("do_put", +[](BFState *s){ char c; std::cin >> c;
                                        s->mem[s->pos] = c; });
        break;
      case '@': // debug
        fun->emit_call("do_debug", +[](BFState *s){
          std::cerr << "pos : " << (int)s->pos << std::endl;
          std::cerr << "mem : " << s->mem << std::endl;
          std::cerr << "val : " << (uint32_t)s->mem[s->pos] << std::endl;
          std::cerr << "---" << std::endl;
        });
        break;
      case '[': // if 0 : jump to ], else call subfun
        branches.emplace(
            fun->emit_add_label(),
            fun->emit_future_label());
        fun->emit_jz(branches.top().second.get());
        break;
      case ']':
        fun->set_future_label(branches.top().second.get());
        fun->emit_jnz(branches.top().first.get());
        if (branches.empty())
          throw std::runtime_error("found ']' but bracket stack is empty");
        branches.pop();
        break;
    }
  }
  if (!branches.empty())
    throw std::runtime_error("a '[' bracket is not closed ");
  fun->emit_ret();
  fun->dump();
  return fun->gen_fun();
}


int main(int argc, const char **argv)
{
  std::unique_ptr<JitterBase> jit = get_jitter(argc, argv);
  std::ifstream iffile(argv[2]);
  std::string program;
  if (!iffile.is_open())
  {
    std::cerr<<"failed to open " << argv[2] << std::endl;
    exit(2);
  }
  try {
    program = get_program(iffile);
  }
  catch (const std::ifstream::failure& e) {
    std::cerr << argv[0] << ": cannot read " << argv[2] << ": \n\t" << e.what() << std::endl;
    exit(2);
  }

  std::vector<uint32_t> mem(256, 0);
  BFState state{mem.data(), 0};

  auto fun = do_jit(jit.get(), program);
  fun->call(&state);
  std::cout << std::endl;
}
