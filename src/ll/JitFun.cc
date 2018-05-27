#include "ll/JitFun.hh"

#include <iostream>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_os_ostream.h>

#include "configs.hh"


namespace ll
{
  JitFun::JitFun(llvm::LLVMContext& cont, llvm::Module* mod, llvm::ExecutionEngine* eengine, llvm::legacy::FunctionPassManager* fpm)
    :cont_(cont), mod_(mod), eengine_(eengine), fpm_(fpm)
  {
    static_assert(sizeof (std::declval<BFState>().pos) == sizeof (uint32_t));
    static_assert(sizeof (std::declval<BFState>().mem[0]) == sizeof (uint32_t));
    llvm::Type * bfstateptrty = llvm::PointerType::get(
        dynamic_cast<llvm::Type*>(llvm::StructType::get(cont_,
            {
              llvm::PointerType::get(llvm::IntegerType::get(cont_, 32), 0),
              llvm::IntegerType::get(cont_, 32)
            }, 0)), 0);
    llvm::FunctionType* ft = llvm::FunctionType::get(
        llvm::Type::getVoidTy(cont_),
        { bfstateptrty }, false);
    fun_ = llvm::Function::Create(ft, llvm::GlobalValue::LinkageTypes::InternalLinkage, "main", mod_);
    bb_ = llvm::BasicBlock::Create(cont_, "entry", fun_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(bb_);


    bfstate_ = fun_->arg_begin(); // only one argument so dont have to iterate
    bfstate_->setName("bfstate");
    zero_ = llvm::ConstantInt::get(cont_, llvm::APInt(32, 0, true));
    one_ = llvm::ConstantInt::get(cont_, llvm::APInt(32, 1, true));
    max_vec_ = llvm::ConstantInt::get(cont_, llvm::APInt(32, configs::vector_size, true));

    llvm::Value *pos = builder_->CreateLoad(builder_->CreateGEP(bfstate_, {zero_, one_}, "posptr"), "posv");
    pos_ = builder_->CreateAlloca(llvm::IntegerType::get(cont_, 32), one_, "pos");
    builder_->CreateStore(pos, pos_);

    llvm::Value *mem = builder_->CreateLoad(builder_->CreateGEP(bfstate_, {zero_, zero_}, "memptr"), "memv");
    mem_ = builder_->CreateAlloca(llvm::PointerType::get(llvm::IntegerType::get(cont_, 32), 0), one_, "mem");
    builder_->CreateStore(mem, mem_);
  }

  JitFun::~JitFun()
  {

  }

  llvm::Value *JitFun::get_pos()
  {
    return builder_->CreateLoad(pos_, "pos");
  }

  void JitFun::set_pos(llvm::Value *val)
  {
    builder_->CreateStore(val, pos_);
  }

  llvm::Value *JitFun::get_val()
  {
    llvm::Value *vptr = builder_->CreateGEP(builder_->CreateLoad(mem_), get_pos(), "valptr");
    return builder_->CreateLoad(vptr, "val");
  }

  void JitFun::set_val(llvm::Value *val)
  {
    llvm::Value *v = builder_->CreateLoad(mem_);
    llvm::Value *vptr = builder_->CreateGEP(v, get_pos(), "valptr");
    builder_->CreateStore(val, vptr);
  }

  void JitFun::emit_add()
  {
    set_val(builder_->CreateAdd(one_, get_val()));
  }

  void JitFun::emit_sub()
  {
    set_val(builder_->CreateSub(get_val(), one_));
  }

  void JitFun::emit_left()
  {
    llvm::Value *pos = get_pos();

    llvm::Value *is_z = builder_->CreateICmpEQ(zero_, pos);
    llvm::Value *posm = builder_->CreateSub(pos, one_);
    set_pos(builder_->CreateSelect(
        is_z,
        pos,
        posm,
        "pos"));
  }

  void JitFun::emit_right()
  {
    // TODO : errors
    llvm::Value *pos = get_pos();


    llvm::Value *is_z = builder_->CreateICmpEQ(max_vec_, pos);
    llvm::Value *posm = builder_->CreateAdd(one_, pos);
    set_pos(builder_->CreateSelect(
        is_z,
        pos,
        posm,
        "pos"));
  }

  void JitFun::emit_call(const char *name, void (*fptr)(BFState*))
  {
    // TODO better way ?
    llvm::Type * bfstateptrty = llvm::PointerType::get(
        dynamic_cast<llvm::Type*>(llvm::StructType::get(cont_,
            {
              llvm::PointerType::get(llvm::IntegerType::get(cont_, 32), 0),
              llvm::IntegerType::get(cont_, 32)
            }, 0)), 0);
    llvm::FunctionType* ft = llvm::FunctionType::get(
        llvm::Type::getVoidTy(cont_),
        bfstateptrty, false);

    llvm::Function* fptrf = llvm::Function::Create(ft,  llvm::GlobalValue::ExternalLinkage, name, mod_);
    eengine_->addGlobalMapping(fptrf, (void*)fptr);
    builder_->CreateCall(ft, fptrf, {bfstate_});
  }

  std::unique_ptr<JitAddr> JitFun::emit_add_label()
  {
    bb_ = llvm::BasicBlock::Create(cont_, "", fun_);
    builder_->CreateBr(bb_); // jump to new block

    builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder
    return std::make_unique<JitBB>(bb_); // return new basic block
  }

  std::unique_ptr<JitAddrU> JitFun::emit_future_label()
  {
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(cont_, "", fun_);
    return std::make_unique<JitBB>(bb);
  }

  void JitFun::set_future_label(JitAddrU* addr)
  {
    if (JitBB* bb = dynamic_cast<JitBB*>(addr))
    {
      builder_->CreateBr(bb->bb_);
      bb_ = bb->bb_;
      builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder
    }
    else
      throw std::runtime_error("libjit: cannot set_future_label: bad JitAddrU");
  }

  void JitFun::emit_jz(JitAddr* addr)
  {
    if (JitBB* bb = dynamic_cast<JitBB*>(addr))
    {
      llvm::Value *is_z = builder_->CreateICmpEQ(zero_, get_val(), "valisz");

      bb_ = llvm::BasicBlock::Create(cont_, "", fun_); // new basic block

      builder_->CreateCondBr(is_z,
          bb->bb_, // True
          bb_// False
          );
      builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder

    }
    else
      throw std::runtime_error("llvm: cannot emit jz: bad JitAddr");
  }

  void JitFun::emit_jz(JitAddrU* addr)
  {
    if (JitBB* bb = dynamic_cast<JitBB*>(addr))
    {
      llvm::Value *is_z = builder_->CreateICmpEQ(zero_, get_val(), "valisz");

      bb_ = llvm::BasicBlock::Create(cont_, "", fun_); // new basic block

      builder_->CreateCondBr(is_z,
          bb->bb_, // True
          bb_// False
          );
      builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder
    }
    else
      throw std::runtime_error("llvm: cannot emit jz: bad JitAddrU");
  }

  void JitFun::emit_jnz(JitAddr* addr)
  {
    if (JitBB* bb = dynamic_cast<JitBB*>(addr))
    {
      llvm::Value *is_z = builder_->CreateICmpEQ(zero_, get_val(), "valisz");

      bb_ = llvm::BasicBlock::Create(cont_, "", fun_); // new basic block

      builder_->CreateCondBr(is_z,
          bb_, // True
          bb->bb_ // False
          );
      builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder

    }
    else
      throw std::runtime_error("llvm: cannot emit jnz: bad JitAddr");
  }

  void JitFun::emit_jnz(JitAddrU* addr)
  {
    if (JitBB* bb = dynamic_cast<JitBB*>(addr))
    {
      llvm::Value *is_z = builder_->CreateICmpEQ(zero_, get_val(), "valisz");

      bb_ = llvm::BasicBlock::Create(cont_, "", fun_); // new basic block

      builder_->CreateCondBr(is_z,
          bb_, // True
          bb->bb_ // False
          );
      builder_ = std::make_unique<llvm::IRBuilder<>>(bb_); // create new builder
    }
    else
      throw std::runtime_error("llvm: cannot emit jnz: bad JitAddrU");
  }

  std::unique_ptr<JittedFun> JitFun::gen_fun()
  {
    static llvm::raw_os_ostream os(std::cerr);
    if (llvm::verifyFunction(*fun_, &os))
      throw std::runtime_error("bad function");
    fpm_->run(*fun_);
    void (*fp)(BFState*) = reinterpret_cast<void(*)(BFState*)>(eengine_->getFunctionAddress("main")); // TODO : reference fun_ instead of main
    return std::make_unique<JittedF>(fp);
  }

  void JitFun::emit_ret()
  {
    builder_->CreateRetVoid();
  }

  void JitFun::dump()
  {
    static llvm::raw_os_ostream os(std::cerr);
    fun_->print(os);
  }

} // namespace ll
