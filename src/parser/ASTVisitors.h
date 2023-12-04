#ifndef ZSCRIPT_ASTVISITORS_H
#define ZSCRIPT_ASTVISITORS_H

#include "AST.h"
#include "CompileError.h"

////////////////////////////////////////////////////////////////
// Standard AST Visitor.

class ASTVisitor
{
public:
    virtual void caseDefault(AST& host, void* param = NULL) {}
	// AST Subclasses
    virtual void caseProgram(ASTProgram& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseFloat(ASTFloat& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseString(ASTString& host, void* param = NULL) {
		caseDefault(host, param);}
	// Statements
    virtual void caseBlock(ASTBlock& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtIf(ASTStmtIf& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL) {
		caseDefault(host, param);}
	virtual void caseStmtSwitch(ASTStmtSwitch& host, void* param = NULL) {
		caseDefault(host, param);}
	virtual void caseSwitchCases(ASTSwitchCases& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtFor(ASTStmtFor& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtWhile(ASTStmtWhile& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtDo(ASTStmtDo& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtReturn(ASTStmtReturn& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtReturnVal(
			ASTStmtReturnVal& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtBreak(ASTStmtBreak& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtContinue(ASTStmtContinue& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStmtEmpty(ASTStmtEmpty& host, void* param = NULL) {
		caseDefault(host, param);}
	// Declarations
    virtual void caseScript(ASTScript& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseImportDecl(ASTImportDecl& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseFuncDecl(ASTFuncDecl& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseDataDeclList(
			ASTDataDeclList& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseDataDecl(ASTDataDecl& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseDataDeclExtraArray(
			ASTDataDeclExtraArray& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseTypeDef(ASTTypeDef& host, void* param = NULL) {
		caseDefault(host, param);}
	// Expressions
    virtual void caseExprConst(ASTExprConst& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprAssign(ASTExprAssign& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprIdentifier(
			ASTExprIdentifier& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprArrow(ASTExprArrow& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprIndex(ASTExprIndex& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprCall(ASTExprCall& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprNegate(ASTExprNegate& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprNot(ASTExprNot& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprBitNot(ASTExprBitNot& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprIncrement(
			ASTExprIncrement& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprPreIncrement(
			ASTExprPreIncrement& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprDecrement(
			ASTExprDecrement& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprPreDecrement(
			ASTExprPreDecrement& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprAnd(ASTExprAnd& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprOr(ASTExprOr& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprGT(ASTExprGT& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprGE(ASTExprGE& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprLT(ASTExprLT& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprLE(ASTExprLE& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprEQ(ASTExprEQ& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprNE(ASTExprNE& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprPlus(ASTExprPlus& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprMinus(ASTExprMinus& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprTimes(ASTExprTimes& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprDivide(ASTExprDivide& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprModulo(ASTExprModulo& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprBitOr(ASTExprBitOr& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprBitXor(ASTExprBitXor& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprLShift(ASTExprLShift& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseExprRShift(ASTExprRShift& host, void* param = NULL) {
		caseDefault(host, param);}
	// Literals
    virtual void caseNumberLiteral(
			ASTNumberLiteral& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseBoolLiteral(
			ASTBoolLiteral& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseStringLiteral(
			ASTStringLiteral& host, void* param = NULL) {
		caseDefault(host, param);}
	virtual void caseArrayLiteral(
			ASTArrayLiteral& host, void* param = NULL) {
		caseDefault(host, param);}
	// Types
	virtual void caseScriptType(ASTScriptType& host, void* param = NULL) {
		caseDefault(host, param);}
    virtual void caseVarType(ASTVarType& host, void* param = NULL) {
		caseDefault(host, param);}
};

////////////////////////////////////////////////////////////////
// AST Visitor that recurses on elements and handles errors.

class RecursiveVisitor : public ASTVisitor, public CompileErrorHandler
{
public:
	RecursiveVisitor() : failure(false), breakNode(NULL) {}
	
	// If any errors have occured.
	bool hasFailed() const {return failure;}

	// Mark as having failed.
	void fail() {failure = true;}
	
	// Used to signal that a compile error has occured.
	void handleError(CompileError const& error, AST const* node, ...);
	
	// Visits a single node. The only virtual visit function as all others
	// defer to this one.
	virtual void visit(AST& node, void* param = NULL);

	////////////////////////////////////////////////////////////////
	// Convenience Functions
	
	// Visits a single node if it exists.
	void visit(AST* node, void* param = NULL);
	// Visit a group of nodes.
	template <class Container>
	void visit(AST& host, Container const& nodes, void* param = NULL)
	{
		for (typename Container::const_iterator it = nodes.begin();
			 it != nodes.end(); ++it)
		{
			if (breakRecursion(host, param)) return;
			visit(**it, param);
		}
	}

	////////////////////////////////////////////////////////////////
	// Cases
	
    virtual void caseDefault(AST&, void*) {}
    virtual void caseProgram(ASTProgram& host, void* param = NULL);
	// Statements
    virtual void caseBlock(ASTBlock& host, void* param = NULL);
    virtual void caseStmtIf(ASTStmtIf& host, void* param = NULL);
    virtual void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL);
	virtual void caseStmtSwitch(ASTStmtSwitch & host, void* param = NULL);
	virtual void caseSwitchCases(ASTSwitchCases & host, void* param = NULL);
    virtual void caseStmtFor(ASTStmtFor& host, void* param = NULL);
    virtual void caseStmtWhile(ASTStmtWhile& host, void* param = NULL);
    virtual void caseStmtDo(ASTStmtDo& host, void* param = NULL);
    virtual void caseStmtReturnVal(
			ASTStmtReturnVal& host, void* param = NULL);
	// Declarations
    virtual void caseScript(ASTScript& host, void* param = NULL);
    virtual void caseFuncDecl(ASTFuncDecl& host, void* param = NULL);
	virtual void caseDataDeclList(ASTDataDeclList& host, void* param = NULL);
	virtual void caseDataDecl(ASTDataDecl& host, void* param = NULL);
	virtual void caseDataDeclExtraArray(
			ASTDataDeclExtraArray& host, void* param = NULL);
    virtual void caseTypeDef(ASTTypeDef&, void* param = NULL);
	// Expressions
	virtual void caseExprConst(ASTExprConst& host, void* param = NULL);
    virtual void caseExprAssign(ASTExprAssign& host, void* param = NULL);
    virtual void caseExprArrow(ASTExprArrow& host, void* param = NULL);
    virtual void caseExprIndex(ASTExprIndex& host, void* param = NULL);
    virtual void caseExprCall(ASTExprCall& host, void* param = NULL);
    virtual void caseExprNegate(ASTExprNegate& host, void* param = NULL);
    virtual void caseExprNot(ASTExprNot& host, void* param = NULL);
    virtual void caseExprBitNot(ASTExprBitNot& host, void* param = NULL);
    virtual void caseExprIncrement(
			ASTExprIncrement& host, void* param = NULL);
    virtual void caseExprPreIncrement(
			ASTExprPreIncrement& host, void* param = NULL);
    virtual void caseExprDecrement(
			ASTExprDecrement& host, void* param = NULL);
    virtual void caseExprPreDecrement(
			ASTExprPreDecrement& host, void* param = NULL);
    virtual void caseExprAnd(ASTExprAnd& host, void* param = NULL);
    virtual void caseExprOr(ASTExprOr& host, void* param = NULL);
    virtual void caseExprGT(ASTExprGT& host, void* param = NULL);
    virtual void caseExprGE(ASTExprGE& host, void* param = NULL);
    virtual void caseExprLT(ASTExprLT& host, void* param = NULL);
    virtual void caseExprLE(ASTExprLE& host, void* param = NULL);
    virtual void caseExprEQ(ASTExprEQ& host, void* param = NULL);
    virtual void caseExprNE(ASTExprNE& host, void* param = NULL);
    virtual void caseExprPlus(ASTExprPlus& host, void* param = NULL);
    virtual void caseExprMinus(ASTExprMinus& host, void* param = NULL);
    virtual void caseExprTimes(ASTExprTimes& host, void* param = NULL);
    virtual void caseExprDivide(ASTExprDivide& host, void* param = NULL);
    virtual void caseExprModulo(ASTExprModulo& host, void* param = NULL);
    virtual void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL);
    virtual void caseExprBitOr(ASTExprBitOr& host, void* param = NULL);
    virtual void caseExprBitXor(ASTExprBitXor& host, void* param = NULL);
    virtual void caseExprLShift(ASTExprLShift& host, void* param = NULL);
    virtual void caseExprRShift(ASTExprRShift& host, void* param = NULL);
	// Literals
    virtual void caseNumberLiteral(
			ASTNumberLiteral& host, void* param = NULL);
	virtual void caseArrayLiteral(ASTArrayLiteral& host, void* param = NULL);

protected:
	// Returns true if we have failed or for some other reason must break out
	// of recursion. Should be called with the current node and param between
	// each action that can fail.
	virtual bool breakRecursion(AST& host, void* param = NULL) const;

	// Current stack of visited nodes.
	vector<AST*> recursionStack;

	// Node which we are breaking recursion until we reach.
	AST* breakNode;
	
	// Set to true if any errors have occured.
	bool failure;
};

#endif

