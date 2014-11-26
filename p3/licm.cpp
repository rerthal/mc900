#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
using namespace llvm;

namespace {
  struct LICM : public LoopPass {
    static char ID;
    LICM() : LoopPass(ID) {}

    bool runOnLoop(Loop *L, LPPassManager &LPM) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<LoopInfo>();
    }
  };
}

bool LICM::runOnLoop(Loop *L, LPPassManager &LPM) {
  return false;
}

char LICM::ID = 0;
static RegisterPass<LICM> X("my-licm", "Loop Invariant Code Motion Pass", false, false);
