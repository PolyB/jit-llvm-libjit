#pragma once

#include "JitterBase.hh"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>

namespace ll
{

class Jitter : public JitterBase
{
  public:
    Jitter();
    ~Jitter() override;

    Jitter(const Jitter&) = delete;
    Jitter(Jitter&&) = delete;
    Jitter& operator=(Jitter&&) = delete;
    Jitter& operator=(const Jitter&) = delete;

    std::unique_ptr<JitFunBase> make_fun() override;
  private:
    llvm::LLVMContext context_;
    llvm::Module* mod_;
    llvm::ExecutionEngine* eengine_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;
};

} // namespace ll
