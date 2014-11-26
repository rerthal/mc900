#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
using namespace llvm;

namespace {
  struct Licm : public LoopPass {
    static char ID;
    Licm() : LoopPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
    }

    bool runOnLoop(Loop *L, LPPassManager &LPM) {
      return false;
    }
  };
}

char Licm::ID = 0;
static RegisterPass<Licm> X("my-licm", "Loop Invariant Code Motion Pass", false, false);
