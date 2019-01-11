#include <queue>
#include <map>
#include <string>

#include <llvm/IR/CFG.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <z3++.h>

#include <assert.h>

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
  std::map<std::string, std::vector<std::pair<std::string, z3::expr>>> predicate_map; // predicate_map[current_bb][pred_bb]: the loop condition from pred_bb to current_bb
  z3::context ctx;
  z3::solver solver;
  BasicBlock * current_bb;
  std::vector<std::string> basic_block_name_list;

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
	  std::cout << "****** call visitFunction: " << getName(F) << std::endl;
	  solver.reset();

	  //std::vector<z3::expr> expr_vec;
	  //for (int i = 0; i < 2; ++ i) expr_vec.push_back(ctx.bv_const("test", 32));
	  //z3::expr test = expr_vec[0];
	  //for (auto i = F.arg_begin(), j = F.arg_end(); i != j; ++ i) {
	  //        Type * type = i->getType();
	  //        auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  //        std::cout << "arg: " << getName(*i) << " size: " << type_size << std::endl;
	  //        z3::expr arg_in_z3 = ctx.bv_val(getName(*i).c_str(), type_size);
	  //}
	  // topological sort 
	  basic_block_name_list.clear();
	  for (auto i = F.begin(), j = F.end(); i != j; ++ i) {
		  BasicBlock & bb = *i;
		  std::string name = getName(bb);
		  int num = 0;
		  for (auto it = pred_begin(&bb), et = pred_end(&bb); it != et; ++ it) {
			  num ++;
		  }
		  num_of_pred[name] = num;
		  //std::cout << "num_of_pred[" << name << "] = " << num << std::endl;
		  basic_block_name_list.push_back(name);
	  }
	  for (std::string name: basic_block_name_list) { // clear the predicate_map 
		  if (predicate_map.find(name) != predicate_map.end()) { 
			  predicate_map[name].clear();
		  }
	  }

	  //std::cout << "topological sort:" << std::endl;
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

	  //std::cout << std::endl;
  }
  void visitBasicBlock(BasicBlock &B) {
	  std::cout << "call visitBasicBlock " << getName(B) << std::endl;
	  std::cout << getName(B) << std::endl;
	  current_bb = &B;
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
		  } else if (isa<ZExtInst>(&inst)) {
			  visitZExt(*dyn_cast<ZExtInst>(&inst));
		  } else if (isa<SExtInst>(&inst)) {
			  visitSExt(*dyn_cast<SExtInst>(&inst));
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

  unsigned getDstSize(Instruction &I) {
	  Type * type = I.getType();
	  unsigned type_size = type->getIntegerBitWidth();
	  return type_size;
  }

  z3::expr getDstExpr(Instruction &I) {
	  auto type_size = getDstSize(I);
	  return ctx.bv_const(getName(I).c_str(), type_size);
  }

  unsigned getOperandSize(Instruction &I, unsigned idx) {
	  Value * value = I.getOperand(idx);
	  Type * type = value->getType();
	  unsigned type_size = type->getIntegerBitWidth();
	  return type_size;
  }

  z3::expr getOperandExpr(Instruction &I, unsigned idx) {
	  Value * value = I.getOperand(idx);
	  auto type_size = getOperandSize(I, idx);
	  std::string name = isa<Constant>(value) ? getName(*value) + "const" : getName(*value);
	  z3::expr z3_expr = ctx.bv_const(name.c_str(), type_size);
	  if (isa<Constant>(value)) {
		  std::string val = getName(*value);
		  int ival = std::atoi(val.c_str());
		  solver.add(z3_expr == ival);
	  }
	  return z3_expr;
  }

  z3::expr getZ3ExprByValue(Value * value) {
	  Type * type = value->getType();
	  unsigned type_size = type->getIntegerBitWidth();
	  std::string name = isa<Constant>(value) ? getName(*value) + "const" : getName(*value);
	  z3::expr z3_expr = ctx.bv_const(name.c_str(), type_size);
	  if (isa<Constant>(value)) {
		  std::string val = getName(*value);
		  int ival = std::atoi(val.c_str());
		  solver.add(z3_expr == ival);
	  }
	  return z3_expr;
  }

  unsigned getValueSize(Value * value) {
	  Type * type = value->getType();
	  unsigned type_size = type->getIntegerBitWidth();
	  return type_size;
  }

  void debugOutput() {
	  std::cout << solver.assertions() << std::endl;
	  if (solver.check() == z3::sat) {
	        std::cout << "Yes" << std::endl;
	  	std::cout << solver.get_model() << std::endl;
	  }
  }

  void visitAdd(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop + rop) == dst);
  }
  void visitSub(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop - rop) == dst);
  }
  void visitMul(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop * rop) == dst);
  }
  void visitShl(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add(shl(lop, rop) == dst);
  }
  void visitLShr(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add(lshr(lop, rop) == dst);
  }
  void visitAShr(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add(ashr(lop, rop) == dst);
  }
  void visitAnd(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop & rop) == dst);
  }
  void visitOr(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop | rop) == dst);
  }
  void visitXor(BinaryOperator &I) {
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);
	  solver.add((lop ^ rop) == dst);
  }

//  void visitSub(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  //auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  auto type_size = type->getIntegerBitWidth();
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " sub " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add((lop - rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitMul(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  //auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  auto type_size = type->getIntegerBitWidth();
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " mul " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add((lop * rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitShl(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  //auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  auto type_size = type->getIntegerBitWidth();
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " shl " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add(shl(lop, rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitLShr(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  //auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  auto type_size = type->getIntegerBitWidth();
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " lshr " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add(lshr(lop, rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitAShr(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " ashr " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add(ashr(lop, rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//
//  }
//  void visitAnd(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " and " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add((lop & rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitOr(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " or " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add((lop | rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }
//  void visitXor(BinaryOperator &I) {
//	  Type * type = I.getType();
//	  auto type_size = data_layout->getTypeAllocSizeInBits(type);
//	  Value * l = I.getOperand(0);
//	  Value * r = I.getOperand(1);
//	  I.dump();
//	  std::cout << getName(I) << " = " << getName(*l) << " xor " << getName(*r) << std::endl;
//
//	  z3::expr dst = ctx.bv_const(getName(I).c_str(), type_size);
//	  std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
//	  std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
//	  z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
//	  z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
//	  solver.add((lop ^ rop) == dst);
//
//	  if (isa<Constant>(l)) {
//	          std::string val = getName(*l);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(lop == ival);
//	  }
//	  if (isa<Constant>(r)) {
//	          std::string val = getName(*r);
//	          int ival = std::atoi(val.c_str());
//	          solver.add(rop == ival);
//	  }
//
//	  if (solver.check() == z3::sat) {
//		  std::cout << "Yes" << std::endl;
//	  	std::cout << solver.get_model() << std::endl;
//	  }
//  }

  void visitZExt(ZExtInst &I) {
	  unsigned dst_size = getDstSize(I);
	  unsigned src_size = getOperandSize(I, 0);
	  z3::expr dst = getDstExpr(I);
	  z3::expr src = getOperandExpr(I, 0);
	  solver.add(dst == zext(src, dst_size - src_size));
  }
  void visitSExt(SExtInst &I) {
	  unsigned dst_size = getDstSize(I);
	  unsigned src_size = getOperandSize(I, 0);
	  z3::expr dst = getDstExpr(I);
	  z3::expr src = getOperandExpr(I, 0);
	  solver.add(dst == sext(src, dst_size - src_size));
	  //debugOutput();
  }

  void visitICmp(ICmpInst &I) {
	  //Value * l = I.getOperand(0);
	  //Value * r = I.getOperand(1);
	  //Type * type = l->getType();
	  //auto type_size = data_layout->getTypeAllocSizeInBits(type);
	  ////std::cout << getName(I) << " = " << getName(*l) << " icmp " << getName(*r) << std::endl; 

	  //z3::expr dst = ctx.bv_const(getName(I).c_str(), 1);
	  //std::string lname = isa<Constant>(l) ? getName(*l) + "const" : getName(*l);
	  //std::string rname = isa<Constant>(r) ? getName(*r) + "const" : getName(*r);
	  //z3::expr lop = ctx.bv_const(lname.c_str(), type_size);
	  //z3::expr rop = ctx.bv_const(rname.c_str(), type_size);
	  //if (isa<Constant>(l)) {
	  //        std::string val = getName(*l);
	  //        int ival = std::atoi(val.c_str());
	  //        solver.add(lop == ival);
	  //}
	  //if (isa<Constant>(r)) {
	  //        std::string val = getName(*r);
	  //        int ival = std::atoi(val.c_str());
	  //        solver.add(rop == ival);
	  //}

	  I.dump();
	  z3::expr dst = getDstExpr(I);
	  z3::expr lop = getOperandExpr(I, 0);
	  z3::expr rop = getOperandExpr(I, 1);

	  ICmpInst::Predicate pred = I.getPredicate();
	  if (pred == ICmpInst::ICMP_EQ) {
		  //std::cout << "ICMP_EQ" << std::endl;
		  solver.add(ite(lop == rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_NE) {
		  //std::cout<< "ICMP_NE" << std::endl;
		  solver.add(ite(lop == rop, dst == 0, dst == 1));
	  } else if (pred == ICmpInst::ICMP_UGT) {
		  //std::cout << "ICMP_UGT" << std::endl;
		  solver.add(ite(ugt(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_UGE) {
		  //std::cout << "ICMP_UGE" << std::endl;
		  solver.add(ite(uge(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_ULT) {
		  //std::cout << "ICMP_ULT" << std::endl;
		  solver.add(ite(ult(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_ULE) {
		  //std::cout << "ICMP_ULE" << std::endl;
		  solver.add(ite(ule(lop, rop), dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SGT) {
		  //std::cout << "ICMP_SGT" << std::endl;
		  solver.add(ite(lop > rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SGE) {
		  //std::cout << "ICMP_SGE" << std::endl;
		  solver.add(ite(lop >= rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SLT) {
		  //std::cout << "ICMP_SLT" << std::endl;
		  solver.add(ite(lop < rop, dst == 1, dst == 0));
	  } else if (pred == ICmpInst::ICMP_SLE) {
		  //std::cout << "ICMP_SLE" << std::endl;
		  solver.add(ite(lop <= rop, dst == 1, dst == 0));
	  }
  }

  z3::expr getPreCondition() {
	  std::string current_bb_name = getName(*current_bb);
	  if (current_bb_name == "entry") {
		  return ctx.bool_val(true);
	  } 
	  //z3::expr_vector args = z3::expr_vector(ctx);
	  //args.push_back(ctx.bool_val(false));
	  z3::expr pred_cond = ctx.bool_val(false);

	  if (predicate_map.find(current_bb_name) != predicate_map.end()) {
	  	std::vector<std::pair<std::string, z3::expr>> & expr_vec = predicate_map[current_bb_name];
		for (auto p: expr_vec) {
			//args.push_back(p.second);
			pred_cond = pred_cond || p.second;
			//std::cout << "From " << p.first << " to " << current_bb_name << ":" << std::endl;
			//std::cout << p.second.to_string() << std::endl; 
		}
	  }
	  return pred_cond;
	  //return z3::mk_or(args);
  }

  void visitBranchInst(BranchInst &I) {
	  std::string current_bb_name = getName(*current_bb);
	  z3::expr pred_cond = getPreCondition();

	  if (I.isUnconditional()) {
		  unsigned num_succ = I.getNumSuccessors();
		  assert(num_succ == 1);
		  BasicBlock * succ_bb = I.getSuccessor(0);
		  std::string succ_bb_name = getName(*succ_bb);
		  predicate_map[succ_bb_name].push_back(std::make_pair(current_bb_name, pred_cond));
		  //predicate_map[succ_bb_name][current_bb_name] = pred_cond;
	  } else {
		  unsigned num_succ = I.getNumSuccessors();
		  assert(num_succ == 2);
		  Value * cond = I.getCondition();
		  z3::expr cond_expr = getZ3ExprByValue(cond);
		  BasicBlock * then_bb = I.getSuccessor(0);
		  BasicBlock * else_bb = I.getSuccessor(1);
		  std::string then_bb_name = getName(*then_bb);
		  std::string else_bb_name = getName(*else_bb);
		  predicate_map[then_bb_name].push_back(std::make_pair(current_bb_name, pred_cond && (cond_expr != 0)));
		  predicate_map[else_bb_name].push_back(std::make_pair(current_bb_name, pred_cond && (cond_expr == 0)));
		  //predicate_map[then_bb_name][current_bb_name] = pred_cond && cond_expr;
		  //predicate_map[else_bb_name][current_bb_name] = pred_cond && (! cond_expr);
	  }
  }
  void visitPHINode(PHINode &I) {
	  std::string current_bb_name = getName(*current_bb);
	  unsigned num_incoming_edges = I.getNumIncomingValues();
	  z3::expr dst_expr = getDstExpr(I);
	  for (unsigned i = 0; i < num_incoming_edges; ++ i) {
		  Value * value = I.getIncomingValue(i);
		  z3::expr value_expr = getZ3ExprByValue(value);
		  BasicBlock * pred_bb = I.getIncomingBlock(i);
		  std::string pred_bb_name = getName(*pred_bb);
		  std::vector<std::pair<std::string, z3::expr>> & vec_expr = predicate_map[current_bb_name];
		  bool found = false;
		  for (auto p: vec_expr) {
			  if (p.first == pred_bb_name) {
				  solver.add(implies(p.second, dst_expr == value_expr));
				  found = true;
				  break;
			  }
		  }
		  assert(found == true);
	  }
	  //debugOutput();
  }

  // Call checkAndReport here.
  void visitGetElementPtrInst(GetElementPtrInst &I) {
	  //TODO
	  I.dump();
	  if (I.isInBounds()) {
		  Type * type = I.getSourceElementType();
		  if (isa<ArrayType>(type)) {
			  ArrayType * array_type = dyn_cast<ArrayType>(type);
			  Type * ele_type = array_type->getElementType();
			  unsigned ele_type_size = ele_type->getIntegerBitWidth();
			  unsigned array_len = array_type->getNumElements();
			  if (ele_type_size == 32) {
				  //std::cout << I.getNumOperands() << std::endl;
				  //for (unsigned i = 0; i < I.getNumOperands(); ++ i) {
				  //        std::cout << getName(*I.getOperand(i)) << " ";
				  //}
				  //std::cout << "\n";
				  Value * idx = I.getOperand(2);
				  z3::expr idx_expr = sext(getZ3ExprByValue(idx), 64 - getValueSize(idx));
				  solver.push();
				  solver.add(! (idx_expr >= ctx.bv_val(0, 64) && idx_expr < ctx.bv_val(array_len, 64)));
				  solver.add(getPreCondition());
				  checkAndReport(solver, I);
				  solver.pop();
			  }
		  }
	  }
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
