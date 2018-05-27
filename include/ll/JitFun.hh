#pragma once

#include "JitFunBase.hh"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>

namespace ll
{

class JitFun : public JitFunBase
{
  public:
    JitFun(llvm::LLVMContext& context, llvm::Module* module, llvm::ExecutionEngine* eengine, llvm::legacy::FunctionPassManager* fpm);
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
    void emit_ret() override;
    std::unique_ptr<JittedFun> gen_fun() override;
    void dump() override;

  private:
    llvm::Value *get_pos();
    void set_pos(llvm::Value *);

    llvm::Value *get_val(); // get mem[pos]
    void set_val(llvm::Value*); // set mem[pos]

    struct JittedF : public JittedFun
    {
        explicit JittedF(void (*fp)(BFState*))
          :fp_(fp)
        {}
        void call(BFState* s) override { fp_(s); }
        void (*fp_)(BFState*);
    };

    struct JitBB : public JitAddr, public JitAddrU
    {
      explicit JitBB(llvm::BasicBlock *bb) : bb_(bb) {}
      llvm::BasicBlock *bb_;
    };

    llvm::LLVMContext& cont_;
    llvm::Module* mod_;
    llvm::ExecutionEngine* eengine_;
    llvm::legacy::FunctionPassManager* fpm_;
    llvm::Function* fun_;
    llvm::BasicBlock* bb_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    llvm::Value *bfstate_;
    llvm::Value *mem_;
    llvm::Value *pos_;

    llvm::Value* zero_;
    llvm::Value* one_;
    llvm::Value* max_vec_;
};

} // namespace ll
