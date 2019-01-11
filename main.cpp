#include <queue>
#include <map>
#include <string>

#include <llvm/IR/CFG.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <z3++.h>

using namespace llvm;

namespace {

// Get unique name of a LLVM node. Applicable to BasicBlock and Instruction.
std::string getName(const Value &Node) {
  if (!Node.getName().empty())
    return Node.getName().str();

  std::string Str;
  raw_string_ostream OS(Str);

  Node.printAsOperand(OS, false);
  return OS.str();
}

// Check
void checkAndReport(z3::solver &solver, const GetElementPtrInst &gep) {
  std::string name = getName(gep);
  std::cout << "Checking with assertions:" << std::endl
            << solver.assertions() << std::endl;
  if (solver.check() == z3::sat)
    std::cout << "GEP " << name << " is potentially out of bound." << std::endl
              << "Model causing out of bound:" << std::endl
              << solver.get_model() << std::endl;
  else
    std::cout << "GEP " << name << " is safe." << std::endl;
}
} // namespace

// ONLY MODIFY THIS CLASS FOR PART 1 & 2!
class Z3Walker : public InstVisitor<Z3Walker> {
private:
  std::map<std::string, std::vector<z3::expr>> predicate_map;
  z3::context ctx;
  z3::solver solver;

  std::map<std::string, int> num_of_pred;
  std::queue<BasicBlock *> bb_queue;
  DataLayout * data_layout;

public:
  Z3Walker() : ctx(), solver(ctx) {}

//  z3::expr getX() {
//	  return ctx.bv_const("x", 32);
//  }
//
//  z3::expr getY() {
//	  return ctx.bv_const("y", 32);
//  }

  // Not using InstVisitor::visit due to their sequential order.
  // We want topological order on the Call Graph and CFG.
  void visitModule(Module &M) {
//	  z3::expr x = getX();
//	  z3::expr y = ctx.bv_const("3", 32);
//	  z3::expr x0 = x;
//	  solver.reset();
//	  solver.add((x & y) == 0);
//	  solver.add(ctx.bv_const("3", 32) == 3);
//	  solver.add(getY() == 2);
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  } else std::cout << "No" << std::endl;
//	  std::cout << solver.get_model() << std::endl;
//	  return ;


	  data_layout = new DataLayout(&M);
	  std::cout << "call visitModule: " << std::endl;
	  for (auto i = M.begin(), fun_end = M.end(); i != fun_end; ++ i) {
		  visitFunction(*i);
	  }
  }
  void visitFunction(Function &F) {
	  std::cout << "call visitFunction: " << getName(F) << std::endl;
	  solver.reset();
	  //for (auto i = F.arg_begin(), j = F.arg_end(); i != j; ++ i) {
	  //        Type * type = i->getType();
	  //        auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  //        std::cout << "arg: " << getName(*i) << " size: " << type_size << std::endl;
	  //        z3::expr arg_in_z3 = ctx.bv_val(getName(*i).c_str(), type_size);
	  //}
	  // topological sort 
	  for (auto i = F.begin(), j = F.end(); i != j; ++ i) {
		  BasicBlock & bb = *i;
		  std::string name = getName(bb);
		  int num = 0;
		  for (auto it = pred_begin(&bb), et = pred_end(&bb); it != et; ++ it) {
			  num ++;
		  }
		  num_of_pred[name] = num;
		  std::cout << "num_of_pred[" << name << "] = " << num << std::endl;
	  }

	  std::cout << "topological sort:" << std::endl;
	  while (! bb_queue.empty()) bb_queue.pop();
	  bb_queue.push(&F.getEntryBlock());
	  while (bb_queue.empty() == false) {
		  BasicBlock * bb = bb_queue.front();
		  bb_queue.pop();
		  //std::cout << getName(*bb) << std::endl;
		  visitBasicBlock(*bb);
		  for (auto it = succ_begin(bb), et = succ_end(bb); it != et; ++ it) {
			  BasicBlock * succ = *it;
			  std::string name = getName(*succ);
			  if ((--num_of_pred[name]) == 0) {
				  bb_queue.push(succ);
			  }
		  }
	  }

	  std::cout << std::endl;
  }
  void visitBasicBlock(BasicBlock &B) {
	  std::cout << "call visitBasicBlock " << getName(B) << std::endl;
	  std::cout << getName(B) << std::endl;
	  for (auto i = B.begin(), j = B.end(); i != j; ++ i) {
		  Instruction & inst = *i;
		  if (isa<BinaryOperator>(&inst)) {
			  visitBinaryOperator(*dyn_cast<BinaryOperator>(&inst));
		  } else if (isa<ICmpInst>(&inst)) {
			  visitICmp(*dyn_cast<ICmpInst>(&inst));
		  } else if (isa<BranchInst>(&inst)) {
			  visitBranchInst(*dyn_cast<BranchInst>(&inst));
		  } else if (isa<PHINode>(&inst)) {
			  visitPHINode(*dyn_cast<PHINode>(&inst));
		  } else if (isa<GetElementPtrInst>(&inst)) {
			  visitGetElementPtrInst(*dyn_cast<GetElementPtrInst>(&inst));
		  }
	  }
  }


  void visitBinaryOperator(BinaryOperator &I) {
	  auto op_code = I.getOpcode();
	  if (op_code == Instruction::Add) {
		  visitAdd(I);
	  } else if (op_code == Instruction::Sub) {
		  visitSub(I);
	  } else if (op_code == Instruction::Mul) {
		  visitMul(I);
	  } else if (op_code == Instruction::Shl) {
		  visitShl(I);
	  } else if (op_code == Instruction::LShr) {
		  visitLShr(I);
	  } else if (op_code == Instruction::AShr) {
		  visitAShr(I);
	  } else if (op_code == Instruction::And) {
		  visitAnd(I);
	  } else if (op_code == Instruction::Or) {
		  visitOr(I);
	  } else if (op_code == Instruction::Xor) {
		  visitXor(I);
	  }
  }

  void visitAdd(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " add " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop + rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitSub(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " sub " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop - rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitMul(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " mul " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop * rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitShl(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " shl " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add(shl(lop, rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitLShr(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " lshr " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add(lshr(lop, rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitAShr(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " ashr " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add(ashr(lop, rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }

  }
  void visitAnd(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " and " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop & rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitOr(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " or " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop | rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }
  void visitXor(BinaryOperator &I) {
	  Type * type = I.getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " xor " << getName(*r) << std::endl;

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  solver.add((lop ^ rop) == dst);

	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  if (solver.check() == z3::sat) {
		  std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }

  void visitZExt(ZExtInst & I) {
	  //TODO
  }
  void visitSExt(SExtInst & I) {
	  //TODO
  }

  void visitICmp(ICmpInst &I) {
	  //TODO
	  Value * l = I.getOperand(0);
	  Value * r = I.getOperand(1);
	  Type * type = l->getType();
	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  I.dump();
	  std::cout << getName(I) << " = " << getName(*l) << " icmp " << getName(*r) << std::endl; 

	  z3::expr dst = ctx.bv_const(getName(I).c_str(), 1);
	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  if (isa<Constant>(l)) {
	          std::string val = getName(*l);
	          int ival = std::atoi(val.c_str());
	          solver.add(lop == ival);
	  }
	  if (isa<Constant>(r)) {
	          std::string val = getName(*r);
	          int ival = std::atoi(val.c_str());
	          solver.add(rop == ival);
	  }

	  ICmpInst::Predicate pred = I.getPredicate();
	  if (pred == ICmpInst::ICMP_EQ) {
		  std::cout << "ICMP_EQ" << std::endl;
		  solver.add(ite(lop == rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_NE) {
		  std::cout<< "ICMP_NE" << std::endl;
		  solver.add(ite(lop == rop, dst == 0, dst == 1));
	  } else if (pred == ICmpInst::ICMP_UGT) {
		  std::cout << "ICMP_UGT" << std::endl;
		  solver.add(ite(ugt(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_UGE) {
		  std::cout << "ICMP_UGE" << std::endl;
		  solver.add(ite(uge(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_ULT) {
		  std::cout << "ICMP_ULT" << std::endl;
		  solver.add(ite(ult(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_ULE) {
		  std::cout << "ICMP_ULE" << std::endl;
		  solver.add(ite(ule(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SGT) {
		  std::cout << "ICMP_SGT" << std::endl;
		  solver.add(ite(lop > rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SGE) {
		  std::cout << "ICMP_SGE" << std::endl;
		  solver.add(ite(lop >= rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SLT) {
		  std::cout << "ICMP_SLT" << std::endl;
		  solver.add(ite(lop < rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SLE) {
		  std::cout << "ICMP_SLE" << std::endl;
		  solver.add(ite(lop <= rop, dst == 1, dst == 0));
	  	  //std::cout << solver.assertions() << std::endl;
	  	  //if (solver.check() == z3::sat) {
	  	  //      std::cout << "Yes" << std::endl;
	  	  //	std::cout << solver.get_model() << std::endl;
	  	  //}
	  }

  }

  void visitBranchInst(BranchInst &I) {
	  //TODO
  }
  void visitPHINode(PHINode &I) {
	  //TODO
  }

  // Call checkAndReport here.
  void visitGetElementPtrInst(GetElementPtrInst &I) {
	  //TODO
  }
};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <IR file>\n";
    return 1;
  }

  LLVMContext llvmctx;

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  auto module = parseIRFile(argv[1], Err, llvmctx);
  if (!module) {
    Err.print(argv[0], errs());
    return 1;
  }

  Z3Walker().visitModule(*module);

  return 0;
}
