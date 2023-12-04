#include "../zc_alleg.h"
#include "SemanticAnalyzer.h"
#include "Scope.h"
#include "CompileError.h"

using namespace ZScript;

struct tag {};
void* const SemanticAnalyzer::paramNone = new tag();
void* const SemanticAnalyzer::paramRead = new tag();
void* const SemanticAnalyzer::paramWrite = new tag();
void* const SemanticAnalyzer::paramReadWrite = new tag();

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer(Program& program)
	: deprecateGlobals(false), program(program)
{
	scope = &program.getScope();
	caseProgram(program.getNode());
}

void SemanticAnalyzer::analyzeFunctionInternals(Function& function)
{
	ASTFuncDecl* functionDecl = function.node;

	// Create function scope.
	function.internalScope = scope->makeFunctionChild(function);
	Scope& functionScope = *function.internalScope;
	TypeStore& typeStore = functionScope.getTypeStore();

	// Grab the script.
	Script* script = NULL;
	if (scope->isScript())
		script = &dynamic_cast<ScriptScope*>(scope)->script;

	// Add the parameters to the scope.
	vector<ASTDataDecl*>& parameters = functionDecl->parameters;
	for (vector<ASTDataDecl*>::iterator it = parameters.begin();
	     it != parameters.end(); ++it)
	{
		ASTDataDecl& parameter = **it;
		string const& name = parameter.name;
		DataType type = parameter.resolveType(&functionScope);
		Variable::create(functionScope, parameter, type, *this);
	}

	// If this is the script's run method, add "this" to the scope.
	if (isRun(function))
	{
		DataType thisType = *script->getType().getThisType(typeStore);
		function.thisVar =
			BuiltinVariable::create(functionScope, thisType, "this", *this);
	}

	// Evaluate the function block under its scope and return type.
	DataType oldReturnType = returnType;
	returnType = function.returnType;
	scope = &functionScope;
	visit(*functionDecl, functionDecl->block->statements);
	scope = scope->getParent();
	returnType = oldReturnType;
}

void SemanticAnalyzer::caseProgram(ASTProgram& host, void*)
{
	// Recurse on elements.
	RecursiveVisitor::caseProgram(host);

	vector<Function*> functions;

	// Analyze function internals.
	functions = program.getUserGlobalFunctions();
	for (vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it)
		analyzeFunctionInternals(**it);
	for (vector<Script*>::iterator it = program.scripts.begin();
		 it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		scope = &script.getScope();
		functions = scope->getLocalFunctions();
		for (vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it)
			analyzeFunctionInternals(**it);
		scope = scope->getParent();
	}

}

// Statements

void SemanticAnalyzer::caseBlock(ASTBlock& host, void*)
{
	// Switch to block scope.
	scope = scope->makeChild();

	// Recurse.
	RecursiveVisitor::caseBlock(host);

	// Restore scope.
	scope = scope->getParent();
}

void SemanticAnalyzer::caseStmtIf(ASTStmtIf& host, void*)
{
    RecursiveVisitor::caseStmtIf(host);
    if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
    checkCast(*host.condition->getReadType(typeStore),
              typeStore.getBool(), &host);
}

void SemanticAnalyzer::caseStmtIfElse(ASTStmtIfElse& host, void*)
{
    RecursiveVisitor::caseStmtIfElse(host);
    if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.condition->getReadType(typeStore),
	          typeStore.getBool(), &host);
}

void SemanticAnalyzer::caseStmtSwitch(ASTStmtSwitch& host, void*)
{
	RecursiveVisitor::caseStmtSwitch(host);
	if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.key->getReadType(typeStore),
	          typeStore.getFloat(), &host);
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host, void*)
{
	// Visit children under new scope.
	scope = scope->makeChild();
	RecursiveVisitor::caseStmtFor(host);
	scope = scope->getParent();
    if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.test->getReadType(typeStore),
	          typeStore.getBool(), &host);
}

void SemanticAnalyzer::caseStmtWhile(ASTStmtWhile& host, void*)
{
    RecursiveVisitor::caseStmtWhile(host);
    if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.test->getReadType(typeStore),
	          typeStore.getBool(), &host);
}

void SemanticAnalyzer::caseStmtReturn(ASTStmtReturn& host, void*)
{
    TypeStore& typeStore = program.getTypeStore();
    if (returnType != typeStore.getVoid())
	    handleError(CompileError::FuncBadReturn, &host,
	                returnType.getName().c_str());
}

void SemanticAnalyzer::caseStmtReturnVal(ASTStmtReturnVal& host, void*)
{
    RecursiveVisitor::caseStmtReturnVal(host);
    if (breakRecursion(host)) return;

    TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.value->getReadType(typeStore), returnType, &host);
}

// Declarations

void SemanticAnalyzer::caseTypeDef(ASTTypeDef& host, void*)
{
	// Resolve the base type under current scope.
	host.type->resolve(*scope);
	DataType type = **host.type;
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            type.getName().c_str());
		return;
	}

	// Add type to the current scope under its new name.
	scope->addType(host.name, type, &host);
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Resolve the base type.
	host.baseType->resolve(*scope);
	DataType baseType = **host.baseType;
	if (!baseType.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            baseType.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (baseType == typeStore.getVoid())
	{
		handleError(CompileError::VoidVar, &host);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		handleError(CompileError::RefVar, &host,
		            baseType.getName().c_str());
		return;
	}

	// Recurse on list contents.
	visit(host, host.declarations());
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);
	if (breakRecursion(host)) return;

	// Then resolve the type.
	DataType type = host.resolveType(scope);
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            type.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (type == typeStore.getVoid())
	{
		handleError(CompileError::VoidVar, &host, host.name.c_str());
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		handleError(CompileError::RefVar, &host,
		            (type.getName() + " " + host.name).c_str());
		return;
	}

	// Currently disabled syntaxes:
	if (getArrayDepth(type) > 1)
	{
		handleError(CompileError::UnimplementedFeature, &host,
		            "Nested Array Declarations");
		return;
	}

	// Is it a constant?
	bool isConstant = false;
	if (type.isConst())
	{
		// A constant without an initializer doesn't make sense.
		if (!host.initializer())
		{
			handleError(CompileError::ConstUninitialized, &host);
			return;
		}

		// Inline the constant if possible.
		isConstant = host.initializer()->getCompileTimeValue(this);
	}

	if (isConstant)
	{
		if (scope->getLocalDatum(host.name))
		{
			handleError(CompileError::VarRedef, &host, host.name.c_str());
			return;
		}
		
		long value = *host.initializer()->getCompileTimeValue(this);
		Constant::create(*scope, host, type, value, *this);
	}
	
	else
	{
		if (scope->getLocalDatum(host.name))
		{
			handleError(CompileError::VarRedef, &host, host.name.c_str());
			return;
		}

		Variable::create(*scope, host, type, *this);
	}

	// Special message for deprecated global variables.
	if (scope->varDeclsDeprecated)
		handleError(CompileError::DeprecatedGlobal, &host, host.name.c_str());

	// Check the initializer.
	if (host.initializer())
	{
		// Make sure we can cast the initializer to the type.
		DataType initType =
			*host.initializer()->getReadType(typeStore);
		checkCast(initType, type, &host);
		if (breakRecursion(host)) return;

		// If it's an array, do an extra check for array count and sizes.
		if (type.isArray())
		{
			if (type != initType)
			{
				handleError(CompileError::IllegalCast, &host,
							initType.getName().c_str(),
							type.getName().c_str());
				return;
			}
		}
	}	
}

void SemanticAnalyzer::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);
	if (breakRecursion(host)) return;
	
	// Iterate over sizes.
	for (vector<ASTExpr*>::const_iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
	{
		ASTExpr& size = **it;

		// Make sure each size can cast to float.
		if (!size.getReadType(typeStore)->canCastTo(typeStore.getFloat()))
		{
			handleError(CompileError::NonIntegerArraySize, &host);
			return;
		}

		// Make sure that the size is constant.
		if (!size.getCompileTimeValue(this))
		{
			handleError(CompileError::ExprNotConstant, &host);
			return;
		}
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Resolve the return type under current scope.
	host.returnType->resolve(*scope);
	DataType returnType = **host.returnType;
	if (!returnType.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            returnType.getName().c_str());
		return;
	}

	// Gather the parameter types.
	vector<DataType> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters;
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		DataType type = decl.resolveType(scope);
		if (!type.isResolved())
		{
			handleError(CompileError::UnresolvedType, &decl,
			            type.getName().c_str());
			return;
		}

		// Don't allow void params.
		if (type == typeStore.getVoid())
		{
			handleError(CompileError::FunctionVoidParam, &decl,
			            decl.name.c_str());
			return;
		}

		paramTypes.push_back(type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(
			returnType, host.name, paramTypes, &host);

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		handleError(CompileError::FunctionRedef, &host, host.name.c_str());
		return;
	}

	function->node = &host;
}

void SemanticAnalyzer::caseScript(ASTScript& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	Script& script = *program.getScript(&host);
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = &script.getScope();
	RecursiveVisitor::caseScript(host);
	scope = scope->getParent();
	if (breakRecursion(host)) return;

	// Check for a valid run function.
	vector<Function*> possibleRuns =
		script.getScope().getLocalFunctions("run");
	if (possibleRuns.size() == 0)
	{
		handleError(CompileError::ScriptNoRun, &host, name.c_str());
		if (breakRecursion(host)) return;
	}
	if (possibleRuns.size() > 1)
	{
		handleError(CompileError::TooManyRun, &host, name.c_str());
		if (breakRecursion(host)) return;
	}
	if (possibleRuns[0]->returnType != typeStore.getVoid())
	{
		handleError(CompileError::ScriptRunNotVoid, &host, name.c_str());
		if (breakRecursion(host)) return;
	}
}

// Expressions

void SemanticAnalyzer::caseExprConst(ASTExprConst& host, void*)
{
	RecursiveVisitor::caseExprConst(host);
	if (breakRecursion(host)) return;

	if (!host.getCompileTimeValue())
	{
		handleError(CompileError::ExprNotConstant, &host);
		return;
	}
}

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	visit(host.left, paramWrite);
	if (breakRecursion(host)) return;
	
	visit(host.right, paramRead);
	if (breakRecursion(host)) return;	

	DataType ltype = *host.left->getWriteType(typeStore);
    DataType rtype = *host.right->getReadType(typeStore);

	checkCast(rtype, ltype, &host);
	if (breakRecursion(host)) return;	

	if (ltype.isConst())
		handleError(CompileError::ConstAssign, &host);
	if (breakRecursion(host)) return;	
}

void SemanticAnalyzer::caseExprIdentifier(
		ASTExprIdentifier& host, void* param)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Bind to named variable.
	host.binding = lookupDatum(*scope, host.components);
	if (!host.binding)
	{
		handleError(CompileError::VarUndeclared, &host,
		            host.asString().c_str());
		return;
	}

	// Can't write to a constant.
	if (param == paramWrite || param == paramReadWrite)
	{
		if (host.binding->type == typeStore.getConstFloat())
		{
			handleError(CompileError::LValConst, &host,
						host.asString().c_str());
			return;
		}
	}
}

void SemanticAnalyzer::caseExprArrow(ASTExprArrow& host, void* param)
{
	TypeStore& typeStore = program.getTypeStore();
	
    // Recurse on left.
    visit(host.left);
    if (breakRecursion(host)) return;

	// Grab the left side's class.
    DataType leftType = *host.left->getReadType(typeStore);
    host.leftClass = leftType.getClass();
    if (!host.leftClass)
	{
		handleError(CompileError::ArrowNotPointer, &host);
        return;
	}

	// Find read function.
	if (!param || param == paramRead || param == paramReadWrite)
	{
		host.readFunction = lookupGetter(*host.leftClass, host.right);
		if (!host.readFunction)
		{
			handleError(CompileError::ArrowNoVar, &host,
						(host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		vector<DataType> const& paramTypes = host.readFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 2 : 1)
		    || paramTypes[0] != leftType)
		{
			handleError(CompileError::ArrowNoVar, &host,
						(host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
	}

	// Find write function.
	if (param == paramWrite || param == paramReadWrite)
	{
		host.writeFunction = lookupSetter(*host.leftClass, host.right);
		if (!host.writeFunction)
		{
			handleError(CompileError::ArrowNoVar, &host,
						(host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		vector<DataType> const& paramTypes = host.writeFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 3 : 2)
		    || paramTypes[0] != leftType)
		{
			handleError(CompileError::ArrowNoVar, &host,
						(host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
	}

	if (host.index)
	{
		visit(host.index);
        if (breakRecursion(host)) return;

		checkCast(*host.index->getReadType(typeStore),
		          typeStore.getFloat(), host.index);
        if (breakRecursion(host)) return;
    }
}

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host, void* param)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Arrow handles its own indexing.
	if (host.array->isTypeArrow())
	{
		static_cast<ASTExprArrow&>(*host.array).index = host.index;
		visit(host.array, param);
		host.setVarType(*host.array->getReadType(typeStore));
		return;
	}
	
	RecursiveVisitor::caseExprIndex(host);
	if (breakRecursion(host)) return;

	host.setVarType(*host.array->getReadType(typeStore));

	// The index must be a number.
    if (host.index)
    {
		checkCast(*host.index->getReadType(typeStore),
		          typeStore.getFloat(), host.index);
        if (breakRecursion(host)) return;
    }
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host, void*)
{
	TypeStore& typeStore = program.getTypeStore();
	
	// Cast left.
	ASTExprArrow* arrow = NULL;
	if (host.left->isTypeArrow())
		arrow = static_cast<ASTExprArrow*>(host.left);
	ASTExprIdentifier* identifier = NULL;
	if (host.left->isTypeIdentifier())
		identifier = static_cast<ASTExprIdentifier*>(host.left);
	
	// Don't visit left for identifier, since we don't want to bind to a
	// variable.
	if (!identifier)
	{
		visit(host.left, paramNone);
		if (breakRecursion(host)) return;
	}

	visit(host, host.parameters);
	if (breakRecursion(host)) return;

	// Gather parameter types.
	vector<DataType> parameterTypes;
	if (arrow)
		parameterTypes.push_back(*arrow->left->getReadType(typeStore));
	for (vector<ASTExpr*>::const_iterator it = host.parameters.begin();
		 it != host.parameters.end(); ++it)
		parameterTypes.push_back(*(*it)->getReadType(typeStore));

	// Grab functions with the proper name.
	vector<Function*> functions =
		identifier
		? lookupFunctions(*scope, identifier->components)
		: lookupFunctions(*arrow->leftClass, arrow->right);

	// Filter out invalid functions.
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end();)
	{
		Function& function = **it;

		// Match against parameter count.
		if (function.paramTypes.size() != parameterTypes.size())
		{
			it = functions.erase(it);
			continue;
		}

		// Check parameter types.
		bool parametersMatch = true;
		for (uint32_t i = 0; i < parameterTypes.size(); ++i)
		{
			if (!parameterTypes[i].canCastTo(function.paramTypes[i]))
			{
				parametersMatch = false;
				break;
			}
		}
		if (!parametersMatch)
		{
			it = functions.erase(it);
			continue;
		}

		// Keep function.
		++it;
	}

	// Find function with least number of casts.
	Function* bestFunction = NULL;
	int bestCastCount = parameterTypes.size() + 1;
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end(); ++it)
	{
		// Count number of casts.
		Function& function = **it;
		int castCount = 0;
		for (uint32_t i = 0; i < parameterTypes.size(); ++i)
			if (parameterTypes[i] != function.paramTypes[i]) ++castCount;

		// If this beats the record, keep it.
		if (castCount < bestCastCount)
		{
			bestFunction = &function;
			bestCastCount = castCount;
		}

		// If this just matches the record, drop the current function.
		else if (castCount == bestCastCount)
			bestFunction = NULL;
	}

	// We failed.
	if (!bestFunction)
	{
		CompileError const& error =
			bestCastCount == parameterTypes.size() + 1
			? CompileError::NoFuncMatch
			: CompileError::TooFuncMatch;
		Function::Signature signature(host.left->asString(), parameterTypes);
		handleError(error, &host, signature.asString().c_str());
		return;
	}

	host.binding = bestFunction;
}

void SemanticAnalyzer::caseExprNegate(ASTExprNegate& host, void*)
{
	analyzeUnaryExpr(host, program.getTypeStore().getFloat());
}

void SemanticAnalyzer::caseExprNot(ASTExprNot& host, void*)
{
	analyzeUnaryExpr(host, program.getTypeStore().getBool());
}

void SemanticAnalyzer::caseExprBitNot(ASTExprBitNot& host, void*)
{
	analyzeUnaryExpr(host, program.getTypeStore().getFloat());
}

void SemanticAnalyzer::caseExprIncrement(ASTExprIncrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprPreIncrement(ASTExprPreIncrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprDecrement(ASTExprDecrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprPreDecrement(ASTExprPreDecrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprAnd(ASTExprAnd& host, void*)
{
	DataType tBool = program.getTypeStore().getBool();
	analyzeBinaryExpr(host, tBool, tBool);
}

void SemanticAnalyzer::caseExprOr(ASTExprOr& host, void*)
{
	DataType tBool = program.getTypeStore().getBool();
	analyzeBinaryExpr(host, tBool, tBool);
}

void SemanticAnalyzer::caseExprGT(ASTExprGT& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprGE(ASTExprGE& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprLT(ASTExprLT& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprLE(ASTExprLE& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprEQ(ASTExprEQ& host, void*)
{
	RecursiveVisitor::caseExprEQ(host);
	if (breakRecursion(host)) return;

	TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.right->getReadType(typeStore),
	          *host.left->getReadType(typeStore));
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprNE(ASTExprNE& host, void*)
{
	RecursiveVisitor::caseExprNE(host);
	if (breakRecursion(host)) return;

	TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.right->getReadType(typeStore),
	          *host.left->getReadType(typeStore));
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprPlus(ASTExprPlus& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprMinus(ASTExprMinus& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprTimes(ASTExprTimes& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprDivide(ASTExprDivide& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprModulo(ASTExprModulo& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprBitAnd(ASTExprBitAnd& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprBitOr(ASTExprBitOr& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprBitXor(ASTExprBitXor& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprLShift(ASTExprLShift& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

void SemanticAnalyzer::caseExprRShift(ASTExprRShift& host, void*)
{
	DataType tFloat = program.getTypeStore().getFloat();
	analyzeBinaryExpr(host, tFloat, tFloat);
}

// Literals

void SemanticAnalyzer::caseStringLiteral(ASTStringLiteral& host, void*)
{
	// Assign type.
	TypeStore& typeStore = program.getTypeStore();
	DataType type = typeStore.getArrayOf(typeStore.getFloat());
	host.setVarType(type);

	// Add to scope as a managed literal.
	Literal::create(*scope, host, type, *this);
}

void SemanticAnalyzer::caseArrayLiteral(ASTArrayLiteral& host, void*)
{
	RecursiveVisitor::caseArrayLiteral(host);
	if (breakRecursion(host)) return;

	// Check that we have some way to determine type.
	if (host.elements.size() == 0 && !host.type)
	{
		handleError(CompileError::EmptyArrayLiteral, &host);
		return;
	}

	TypeStore& typeStore = program.getTypeStore();
	// If present, type check the explicit size.
	if (host.size)
	{
		checkCast(*host.size->getReadType(typeStore),
		          typeStore.getFloat(), host.size);
		if (breakRecursion(host)) return;
	}

	// Don't allow an explicit size if we're part of a declaration.
	if (host.size && host.declaration)
	{
		handleError(CompileError::ArrayLiteralResize, &host);
		return;
	}

	// If present, resolve the explicit type.
	if (host.type)
	{
		host.type->resolve(*scope);
		DataType elementType = **host.type;
		if (!elementType.isResolved())
		{
			handleError(CompileError::UnresolvedType, &host,
			            elementType.getName().c_str());
			return;
		}

		// Disallow void type.
		if (elementType == typeStore.getVoid())
		{
			handleError(CompileError::VoidArr, &host);
			return;
		}

		// Convert to array type.
		host.setReadType(typeStore.getArrayOf(elementType));
	}

	// Otherwise, grab the type from the first element.
	else
	{
		host.setReadType(
				typeStore.getArrayOf(
						*host.elements[0]->getReadType(typeStore)));
	}

	// If initialized, check that each element can be cast to type.
	for (vector<ASTExpr*>::iterator it = host.elements.begin();
		 it != host.elements.end(); ++it)
	{
		ASTExpr& element = **it;
		checkCast(*element.getReadType(typeStore),
				  *host.getReadType(typeStore)->getElementType(), &host);
		if (breakRecursion(host)) return;
	}
	
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(typeStore), *this);
}

void SemanticAnalyzer::checkCast(
		DataType const& sourceType, DataType const& targetType, AST* node)
{
	if (sourceType.canCastTo(targetType)) return;
	handleError(CompileError::IllegalCast, node,
				sourceType.getName().c_str(),
				targetType.getName().c_str());
}

void SemanticAnalyzer::analyzeUnaryExpr(
		ASTUnaryExpr& host, DataType const& type)
{
	visit(host.operand);
	if (breakRecursion(host)) return;

	TypeStore& typeStore = program.getTypeStore();
	checkCast(*host.operand->getReadType(typeStore), type, &host);
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeIncrement(ASTUnaryExpr& host)
{
    visit(host.operand, paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow& arrow =
			operand.isTypeArrow()
			? static_cast<ASTExprArrow&>(operand)
			: static_cast<ASTExprArrow&>(
					*static_cast<ASTExprIndex&>(operand).array);
    }

    TypeStore& typeStore = program.getTypeStore();
    checkCast(*operand.getReadType(typeStore),
              typeStore.getFloat(), &host);
    if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeBinaryExpr(
		ASTBinaryExpr& host, DataType const& leftType,
		DataType const& rightType)
{
	TypeStore& typeStore = program.getTypeStore();
	
	visit(host.left);
	if (breakRecursion(host)) return;
	checkCast(*host.left->getReadType(typeStore), leftType, &host);
	if (breakRecursion(host)) return;

	visit(host.right);
	if (breakRecursion(host)) return;
	checkCast(*host.right->getReadType(typeStore), rightType, &host);
	if (breakRecursion(host)) return;
}

