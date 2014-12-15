#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  struct LICM : public LoopPass {
    static char ID;
    LICM() : LoopPass(ID) {}
    bool runOnLoop(Loop *L, LPPassManager &LPM) override;
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequiredID(LoopSimplifyID);
    };
  };
}

bool LICM::runOnLoop(Loop *loop, LPPassManager &LPM) {
  Loop::block_iterator block;
  BasicBlock::iterator instruction;
  std::vector<Instruction *> loopInvariants;
  bool changedOnLoop, instructionIsLoopInvariant;

  do {
    changedOnLoop = false;
    for (block = loop->block_begin(); block != loop->block_end(); ++block) {
      for (instruction = (*block)->begin(); instruction != (*block)->end(); ++instruction) {
        instructionIsLoopInvariant = true;

        for (int i = 0; i < loopInvariants.size(); i++) {
          if (loopInvariants[i] == instruction) instructionIsLoopInvariant = false;
        }

        for (int i = 0; i < instruction->getNumOperands(); i++) {
          Instruction * ins = dyn_cast<Instruction>(instruction->getOperand(i));
          Constant    * con = dyn_cast<Constant>(instruction->getOperand(i));
          Argument    * arg = dyn_cast<Argument>(instruction->getOperand(i));
          if (ins) {
            bool belongsToLoop   = false;
            bool isLoopInvariant = false;
            for (Loop::block_iterator childBlock = loop->block_begin(); childBlock != loop->block_end(); ++childBlock) {
              if (ins->getParent() == *childBlock) belongsToLoop = true;
            }
            for (int i = 0; i < loopInvariants.size(); i++) {
              if (loopInvariants[i] == ins) isLoopInvariant = true;
            }
            if (belongsToLoop && !isLoopInvariant) instructionIsLoopInvariant = false;
          } else if (!con && !arg) {
            instructionIsLoopInvariant = false;
          }
        }
        if (instructionIsLoopInvariant) {
          loopInvariants.push_back(instruction);
          changedOnLoop = true;
        }
      }
    }
  } while (changedOnLoop);

  BasicBlock * PreHeader = loop->getLoopPreheader();
  DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SmallVector<BasicBlock *, 8> exitBlocks;
  loop->getExitBlocks(exitBlocks);
  bool changed = false;

  for (int i = 0; i < loopInvariants.size(); i++) {
    bool dominates = true;
    for (int j = 0; j < exitBlocks.size(); j ++) {
      if (!DT->dominates(loopInvariants[i]->getParent(), exitBlocks[j])) {
        dominates = false;
      }
    }
    if (dominates) {
      changed = true;
      loopInvariants[i]->moveBefore(PreHeader->getTerminator());
    }
  }
  return changed;
}

char LICM::ID = 0;
static RegisterPass<LICM> X("my-licm", "Loop Invariant Code Motion Pass", false, false);
