#ifndef DTL_AST_HPP
#define DTL_AST_HPP

#include <ostream>
#include <sstream>
#include <string.h>
#include <list>
#include "position.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "errors.hpp"
#include "type_analysis.hpp"
#include <vector>
#include <fstream>




namespace DTL
{
	// Optimization passes
	class ConstantFoldPass;
	class ConstantPropagationPass;
	class ConstantCoalescePass;
	class DeadCodeEliminationPass;
	// standard passes
	class TypeAnalysis;
	class ResourceAnalysis;
	class ResourceAllocation;
	// pass resources
	class Opd;
	class SymbolTable;
	class SemSymbol;
	// forward node declarations
	class DeclNode;
	class VarDeclNode;
	class StmtNode;
	class FormalDeclNode;
	class TypeNode;
	class ExpNode;
	class IDNode;
	class ForStmtNode;
	class IntLitNode;

	enum NODETAG
	{
		PROGRAMNODE,
		IDNODE,
		CONSTDECLNODE,
		POSTINCNODE,
		FORSTMTNODE,
		TYPENODE,
		TIMESNODE,
		PLUSNODE,
		LESSNODE,
		LESSEQNODE,
		INTLITNODE,
		INTTYPENODE,
		OUTSTMTNODE,
		CONSTARRAYDECLNODE,
		ARRAYINDEXNODE,
		UNARYEXPNODE
	};

	class ASTNode
	{
	public:
		ASTNode(const Position *pos) : myPos(pos) {}
		// virtual void unparse(std::ostream&, int) = 0;
		const Position *pos() { return myPos; }
		std::string posStr() { return pos()->span(); }
		std::string Check() { return "CHECK\n"; }
		virtual bool nameAnalysis(SymbolTable *symTab) = 0;
		NODETAG getTag() const { return myTag; }
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) = 0;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) = 0;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) = 0;
		// Note that there is no ASTNode::typeAnalysis. To allow
		//  for different type signatures, type analysis is
		//  implemented as needed in various subclasses
		NODETAG myTag;

	protected:
		const Position *myPos = nullptr;
	};

	class ProgramNode : public ASTNode
	{
	public:
		ProgramNode(std::vector<StmtNode *> globalStatements)
			: ASTNode(nullptr), myStatements(globalStatements)
		{
			myTag = NODETAG::PROGRAMNODE;
		}
		// void unparse(std::ostream&, int) override;
		virtual bool nameAnalysis(SymbolTable *) override;
		virtual void typeAnalysis(TypeAnalysis *ta);
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) { return ""; };
		virtual void PrintAST(const std::string &file);
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;


		/*
			Optimization methods
		*/
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;

		// IRProgram * to3AC(TypeAnalysis * ta);
		virtual ~ProgramNode() {}

	private:
		std::vector<StmtNode *> myStatements;
	};

	class ExpNode : public ASTNode
	{
	protected:
		ExpNode(const Position *p) : ASTNode(p) {}

	public:
		// virtual void unparseNested(std::ostream& out);
		// virtual void unparse(std::ostream& out, int indent) override = 0;
		virtual bool nameAnalysis(SymbolTable *symTab) override = 0;
		virtual void typeAnalysis(TypeAnalysis *ta) = 0;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) = 0;
		virtual int GetMaxDepth() = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) = 0;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) = 0;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) = 0;

		// virtual Opd * flatten(Procedure * proc) = 0;
	};

	class LocNode : public ExpNode
	{
	public:
		LocNode(const Position *p) : ExpNode(p) {} //, //mySymbol(nullptr){}
		void attachSymbol(SemSymbol *symbolIn) { mySymbol = symbolIn; };
		SemSymbol *getSymbol() { return mySymbol; }
		virtual bool nameAnalysis(SymbolTable *symTab) override = 0;
		virtual void typeAnalysis(TypeAnalysis *) override = 0;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) = 0;
		virtual int GetMaxDepth() = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) = 0;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) = 0;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) = 0;

		// virtual Opd * flatten(Procedure * proc) override = 0;
	private:
		SemSymbol *mySymbol;
	};
	class StmtNode : public ASTNode
	{
	public:
		StmtNode(const Position *p) : ASTNode(p) {}
		virtual bool nameAnalysis(SymbolTable *symTab) override = 0;
		// virtual void unparse(std::ostream& out, int indent) override = 0;
		virtual void typeAnalysis(TypeAnalysis *) = 0;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) = 0;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) = 0;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) = 0;

		int GetMaxDepth() { return 0; }
		// virtual void to3AC(Procedure * proc) = 0;
	};

	class DeclNode : public StmtNode
	{
	public:
		DeclNode(const Position *p) : StmtNode(p) {}
		// void unparse(std::ostream& out, int indent) override =0;
		// virtual std::string getName() = 0;
		virtual bool nameAnalysis(SymbolTable *symTab) override = 0;
		virtual void typeAnalysis(TypeAnalysis *) override = 0;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) = 0;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) = 0;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) = 0;

		// virtual void to3AC(IRProgram * prog) = 0;
		// virtual void to3AC(Procedure * proc) override = 0;
	};

	/*
		We treat this as a generic VarDecl, not a constant. We can change the name later
	*/
	class ConstDeclNode : public DeclNode
	{
	public:
		ConstDeclNode(const Position *p, TypeNode *type, IDNode *id, IntLitNode *assignval) : DeclNode(p), myType(type), myID(id), myVal(assignval)
		{
			myTag = NODETAG::CONSTDECLNODE;
			m_Opt = true;
		}
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *) override;
		virtual bool nameAnalysis(SymbolTable *symTab) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags);
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;

		std::string GetIDString() const;
		void SetOpt(bool opt) {m_Opt = opt;}
		bool GetOpt() {return m_Opt;}
	private:
		bool m_Opt;
		TypeNode *myType;
		IDNode *myID;
		IntLitNode *myVal;
	};


	class ConstArrayDeclNode : public DeclNode
	{
	public:
		ConstArrayDeclNode(const Position *p, TypeNode *type, IDNode *id, std::vector<IntLitNode*> assignVals) : DeclNode(p), myType(type), myID(id), myVals(assignVals)
		{
			myTag = NODETAG::CONSTARRAYDECLNODE;
		}
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *) override;
		virtual bool nameAnalysis(SymbolTable *symTab) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags);
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		std::string GetIDString() const;
	private:
		TypeNode *myType;
		IDNode *myID;
		std::vector<IntLitNode*> myVals;
	};



	class PostIncStmtNode : public StmtNode
	{
	public:
		PostIncStmtNode(const Position *p, LocNode *inLoc)
			: StmtNode(p), myLoc(inLoc) { myTag = NODETAG::POSTINCNODE; }
		// void unparse(std::ostream& out, int indent) override;
		virtual bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth();

		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		// virtual void to3AC(Procedure * prog) override;
	private:
		LocNode *myLoc;
	};

	class ForStmtNode : public StmtNode
	{
	public:
		ForStmtNode(const Position *p, StmtNode *init, ExpNode *condition, StmtNode *updateStmt, std::vector<StmtNode *> &innerStatements)
			: StmtNode(p), myInit(init), myCondExp(condition), myUpdateStmt(updateStmt), myStatements(innerStatements)
		{
			myTag = NODETAG::FORSTMTNODE;
		}
		// void unparse(std::ostream& out, int indent) override;
		virtual bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int Collapse(ResourceAllocation *ralloc) override;
		std::string GetInitVar() const {return static_cast<ConstDeclNode*>(myInit)->GetIDString();}
		// virtual void to3AC(Procedure * prog) override;

	private:
		int RegInitValue;
		int RegMaxValue;

		StmtNode *myInit;
		ExpNode *myCondExp;
		StmtNode *myUpdateStmt;
		std::vector<StmtNode *> myStatements;
	};

	class OutStmtNode : public StmtNode
	{
	public:
		OutStmtNode(const Position *p, ExpNode *inExp)
			: StmtNode(p), myExp(inExp) { myTag = NODETAG::OUTSTMTNODE; }
		// void unparse(std::ostream& out, int indent) override;
		virtual bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags);
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags);
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual int GetMaxDepth();

		int maxDepth; // calling GetMaxDepth() will also set this value

		// virtual void to3AC(Procedure * prog) override;
	private:
		ExpNode *myExp;
	};

	class TypeNode : public ASTNode
	{
	public:
		TypeNode(const Position *p) : ASTNode(p) { myTag = NODETAG::TYPENODE; }
		// void unparse(std::ostream&, int) override = 0;
		virtual const DataType *getType() const = 0;
		virtual bool nameAnalysis(SymbolTable *) override { return true; }
		virtual void typeAnalysis(TypeAnalysis *) { return; };
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags);
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass);
	};

	class IntTypeNode : public TypeNode
	{
	public:
		IntTypeNode(const Position *p) : TypeNode(p) {
			myTag = NODETAG::INTTYPENODE;
		}

		virtual const DataType *getType() const { return BasicType::produce(BaseType::INT); }
		// virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
		// void unparse(std::ostream& out, int indent) override;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc) override;
		

		// virtual const DataType * getType() const override;
	};


	class ArrayIndexNode : public LocNode
	{
	public:
		ArrayIndexNode(const Position *p, IDNode* id, IDNode* index_id)
			: myID(id), myIndexVar(index_id), LocNode(p) { myTag = NODETAG::ARRAYINDEXNODE; }
		//std::string getName() { return name; }
		// void unparse(std::ostream& out, int indent) override;
		// void unparseNested(std::ostream& out) override;
		bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth();
		// virtual Opd * flatten(Procedure * proc) override;
	private:
		IDNode* myID;
		IDNode* myIndexVar;
	};

	

	class IDNode : public LocNode
	{
	public:
		IDNode(const Position *p, std::string nameIn)
			: LocNode(p), name(nameIn) { myTag = NODETAG::IDNODE; }
		std::string getName() { return name; }
		// void unparse(std::ostream& out, int indent) override;
		// void unparseNested(std::ostream& out) override;
		bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) { return; };
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc) override;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth();
		// virtual Opd * flatten(Procedure * proc) override;
	private:
		std::string name;
	};

	class UnaryExpNode : public ExpNode
	{
	public:
		UnaryExpNode(const Position *p, ExpNode *expIn)
			: ExpNode(p)
		{
			this->myExp = expIn;
			myTag = NODETAG::UNARYEXPNODE;
		}
		// virtual void unparse(std::ostream& out, int indent) override = 0;
		virtual bool nameAnalysis(SymbolTable *symTab) override = 0;
		virtual void typeAnalysis(TypeAnalysis *) override = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TranformPass() = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth() = 0;
		// virtual Opd * flatten(Procedure * prog) override = 0;
	protected:
		ExpNode *myExp;
	};
	class IntLitNode : public ExpNode
	{
	public:
		IntLitNode(const Position *p, int numIn)
			: ExpNode(p), myNum(numIn) 
			{
				myTag = NODETAG::INTLITNODE;
			}
		// virtual void unparseNested(std::ostream& out) override{
		//	unparse(out, 0);
		// }
		virtual bool nameAnalysis(SymbolTable *symTab) override { return true; }
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		// void unparse(std::ostream& out, int indent) override;
		// bool nameAnalysis(SymbolTable * symTab) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass) override;
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth();
		int GetVal() const {return myNum;}
		// virtual Opd * flatten(Procedure * prog) override;
		int myNum;
	private:
		
	};

	class BinaryExpNode : public ExpNode
	{
	public:
		BinaryExpNode(const Position *p, ExpNode *lhs, ExpNode *rhs)
			: ExpNode(p), myExp1(lhs), myExp2(rhs) {}
		bool nameAnalysis(SymbolTable *symTab) override;
		virtual void typeAnalysis(TypeAnalysis *) override = 0;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) = 0;
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override = 0;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth) = 0;
		virtual int Collapse(ResourceAllocation *ralloc) = 0;
		virtual ASTNode *TransformPass(uint8_t opt_flags) = 0;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) = 0;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) = 0;
		virtual ASTNode *ConstPropagation(DTL::ConstantPropagationPass* prop_pass);
		virtual ASTNode *ConstCoalesce(DTL::ConstantCoalescePass* coalesce_pass, int pass) override;
		virtual ASTNode *DeadCodeElimination(DTL::DeadCodeEliminationPass* elim_pass, int pass) override;
		virtual int GetMaxDepth() = 0;
		// virtual Opd * flatten(Procedure * prog) override = 0;
	protected:
		ExpNode *myExp1;
		ExpNode *myExp2;
		// void binaryLogicTyping(TypeAnalysis * typing);
		// void binaryEqTyping(TypeAnalysis * typing);
		// void binaryRelTyping(TypeAnalysis * typing);
		// void binaryMathTyping(TypeAnalysis * typing);
	};

	class PlusNode : public BinaryExpNode
	{
	public:
		PlusNode(const Position *p, ExpNode *e1, ExpNode *e2)
			: BinaryExpNode(p, e1, e2), IsPassThrough(false) { myTag = NODETAG::PLUSNODE; }
		// virtual bool nameAnalysis(SymbolTable * symTab) override;
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc);
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual int GetMaxDepth();
		bool isPassThrough();
		void setPassThrough() { IsPassThrough = true; }

	private:
		bool IsPassThrough;
		// virtual Opd * flatten(Procedure * prog) override;
	};
	class TimesNode : public BinaryExpNode
	{
	public:
		TimesNode(const Position *p, ExpNode *e1In, ExpNode *e2In)
			: BinaryExpNode(p, e1In, e2In) { myTag = NODETAG::TIMESNODE; }
		// virtual bool nameAnalysis(SymbolTable * symTab) override;
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer);
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;
		virtual int Collapse(ResourceAllocation *ralloc);
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual int GetMaxDepth();
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		// virtual Opd * flatten(Procedure * prog) override;
	};

	class LessNode : public BinaryExpNode
	{
	public:
		LessNode(const Position *p, ExpNode *e1, ExpNode *e2)
			: BinaryExpNode(p, e1, e2) { myTag = NODETAG::LESSNODE; }

		// virtual bool nameAnalysis(SymbolTable * symTab) override;
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *ta) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) { return; };
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override
		{
			node_num++;
			std::string name = "LessNode" + std::to_string(node_num);
			auto exp1NodeString = myExp1->PrintAST(node_num, outfile);
			auto exp2NodeString = myExp2->PrintAST(node_num, outfile);

			outfile << name << " -> " << exp1NodeString << ";\n";
			outfile << name << " -> " << exp2NodeString << ";\n";

			return name;
		}
		virtual int Collapse(ResourceAllocation *ralloc);
		virtual int GetMaxDepth();
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		// virtual Opd * flatten(Procedure * proc) override;
	};

	class LessEqNode : public BinaryExpNode
	{
	public:
		LessEqNode(const Position *pos, ExpNode *e1, ExpNode *e2)
			: BinaryExpNode(pos, e1, e2) { myTag = NODETAG::LESSEQNODE; }
		virtual std::string PrintAST(int &node_num, std::ofstream &outfile) override;

		// virtual bool nameAnalysis(SymbolTable * symTab) override;
		// void unparse(std::ostream& out, int indent) override;
		virtual void typeAnalysis(TypeAnalysis *) override;
		virtual void resourceAnalysis(ResourceAnalysis *ra, int layer) { return; };
		virtual void resourceAllocation(ResourceAllocation *ralloc, int depth);
		virtual int Collapse(ResourceAllocation *ralloc);
		virtual ASTNode *TransformPass(uint8_t opt_flags) override;
		virtual ASTNode *TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags) override;
		virtual int GetMaxDepth();
		virtual ASTNode *ConstFold(DTL::ConstantFoldPass* foldpass) override;
		// virtual Opd * flatten(Procedure * prog) override;
	};

}

#endif
