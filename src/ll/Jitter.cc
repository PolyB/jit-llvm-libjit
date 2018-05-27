#include "ll/Jitter.hh"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include "ll/JitFun.hh"

namespace ll
{

  Jitter::Jitter()
    :context_()
  {
    std::string errstr;

    llvm::InitializeNativeTarget();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    auto mod = llvm::make_unique<llvm::Module>("test-jit-llvm", context_);
    mod_ = mod.get();
    eengine_ = llvm::EngineBuilder(std::move(mod)).setErrorStr(&errstr).create();
    if (!eengine_)
      throw std::runtime_error("cannot create engine builder : " + errstr);
    fpm_ = llvm::make_unique<llvm::legacy::FunctionPassManager>(mod_);
    fpm_->add(llvm::createAggressiveDCEPass());
    fpm_->add(llvm::createCFGSimplificationPass());
    fpm_->add(llvm::createConstantPropagationPass());
    fpm_->add(llvm::createDeadCodeEliminationPass());
    fpm_->add(llvm::createGVNPass());
    fpm_->add(llvm::createIndVarSimplifyPass());
    fpm_->add(llvm::createInstructionCombiningPass());
    fpm_->add(llvm::createLICMPass());
    fpm_->add(llvm::createLowerSwitchPass());
    fpm_->add(llvm::createPromoteMemoryToRegisterPass());
    fpm_->add(llvm::createReassociatePass());
    fpm_->doInitialization();
  }

  Jitter::~Jitter()
  {

  }

  std::unique_ptr<JitFunBase> Jitter::make_fun()
  {
    return std::make_unique<JitFun>(context_, mod_, eengine_, fpm_.get());
  }

} // namespace ll
