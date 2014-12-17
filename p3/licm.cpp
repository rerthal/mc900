#include "llvm/Pass.h"
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
      AU.addRequiredID(LoopSimplifyID);
      AU.addPreservedID(LoopSimplifyID);
    };
  };
}

bool LICM::runOnLoop(Loop *loop, LPPassManager &LPM) {
  std::vector<Instruction *> loopInvariants;
  bool changedOnLoop, instructionIsLoopInvariant;
  BasicBlock * PreHeader = loop->getLoopPreheader();

  do {
    changedOnLoop = false;

    for (Loop::block_iterator block = loop->block_begin(); block != loop->block_end(); ++block) {
      for (BasicBlock::iterator instruction = (*block)->begin(); instruction != (*block)->end(); ++instruction) {
        instructionIsLoopInvariant = true;

        for (int i = 0; i < loopInvariants.size(); i++)
          if (loopInvariants[i] == instruction) instructionIsLoopInvariant = false;

        if (instruction->mayReadOrWriteMemory()) break;
        if (!isSafeToSpeculativelyExecute(instruction)) break;

        for (Instruction::op_iterator operand = instruction->op_begin(); operand != instruction->op_end(); ++operand) {
          Instruction * ins = dyn_cast<Instruction>(operand);

          if (ins) {
            bool belongsToLoop = false;
            bool isLoopInvariant = false;

            for (Loop::block_iterator childBlock = loop->block_begin(); childBlock != loop->block_end(); ++childBlock)
              if (ins->getParent() == *childBlock) belongsToLoop = true;

            for (int i = 0; i < loopInvariants.size(); i++)
              if (loopInvariants[i] == ins) isLoopInvariant = true;

            if (belongsToLoop && !isLoopInvariant) instructionIsLoopInvariant = false;

          } else if (!dyn_cast<Constant>(operand) && ! dyn_cast<Argument>(operand)) instructionIsLoopInvariant = false;
        }
        if (instructionIsLoopInvariant) {
          loopInvariants.push_back(instruction);
          changedOnLoop = true;
        }
      }
    }
  } while (changedOnLoop);

  for (int i = 0; i < loopInvariants.size(); i++) loopInvariants[i]->moveBefore(PreHeader->getTerminator());
  return loopInvariants.size() > 0;
}

char LICM::ID = 0;
static RegisterPass<LICM> X("my-licm", "Loop Invariant Code Motion Pass", false, false);
