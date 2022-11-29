//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "../zsyssimple.h"
#include "ByteCode.h"
#include "CompileError.h"
#include "CompileOption.h"
#include "GlobalSymbols.h"
#include "y.tab.hpp"
#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <string>
#include <memory>

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Scope.h"
#include "SemanticAnalyzer.h"
#include "BuildVisitors.h"
#include "RegistrationVisitor.h"
#include "ZScript.h"
using std::unique_ptr;
using std::shared_ptr;
using namespace ZScript;

extern std::vector<string> ZQincludePaths;
//#define PARSER_DEBUG

ScriptsData* compile(string const& filename);

#if PARSER_DEBUG < 0
int32_t main(int32_t argc, char *argv[])
{
	if (argc < 2) return -1;
	compile(string(argv[1]));
}
#endif

void ScriptParser::initialize()
{
	vid = 0;
	fid = 0;
	gid = 1;
	lid = 0;
	CompileError::initialize();
	CompileOption::initialize();
	includePaths.clear();
	includePaths.resize(0);
}
extern uint32_t zscript_failcode;
extern bool zscript_had_warn_err;
extern bool zscript_error_out;
unique_ptr<ScriptsData> ZScript::compile(string const& filename)
{
	zscript_failcode = 0;
	zscript_had_warn_err = false;
	zscript_error_out = false;
	ScriptParser::initialize();
	
	zconsole_info("%s", "Pass 1: Parsing");

	unique_ptr<ASTFile> root(parseFile(filename, true));
	if(zscript_error_out) return nullptr;
	if (!root.get())
	{
		log_error(CompileError::CantOpenSource(NULL));
		return nullptr;
	}

	zconsole_info("%s", "Pass 2: Preprocessing");

	if (!ScriptParser::preprocess(root.get(), ScriptParser::recursionLimit))
		return nullptr;
	if(zscript_error_out) return nullptr;

	SimpleCompileErrorHandler handler;
	Program program(*root, &handler);
	if (handler.hasError())
		return nullptr;
	if(zscript_error_out) return nullptr;

	zconsole_info("%s", "Pass 3: Registration");

	RegistrationVisitor regVisitor(program);
	if(regVisitor.hasFailed()) return nullptr;
	if(zscript_error_out) return nullptr;

	zconsole_info("%s", "Pass 4: Analyzing Code");

	SemanticAnalyzer semanticAnalyzer(program);
	if (semanticAnalyzer.hasFailed() || regVisitor.hasFailed())
		return nullptr;
	if(zscript_error_out) return nullptr;

	FunctionData fd(program);
	if(zscript_error_out) return nullptr;
	if (fd.globalVariables.size() > MAX_SCRIPT_REGISTERS)
	{
		log_error(CompileError::TooManyGlobal(NULL));
		return nullptr;
	}

	zconsole_info("%s", "Pass 5: Generating object code");

	unique_ptr<IntermediateData> id(ScriptParser::generateOCode(fd));
	if (!id.get())
		return nullptr;
	if(zscript_error_out) return nullptr;
	
	zconsole_info("%s", "Pass 6: Assembling");

	ScriptParser::assemble(id.get());

	unique_ptr<ScriptsData> result(new ScriptsData(program));
	if(zscript_error_out) return nullptr;

	zconsole_info("%s", "Success!");

	return unique_ptr<ScriptsData>(result.release());
}

int32_t ScriptParser::vid = 0;
int32_t ScriptParser::fid = 0;
int32_t ScriptParser::gid = 1;
int32_t ScriptParser::lid = 0;

string ScriptParser::prepareFilename(string const& filename)
{
	string retval = filename;

	regulate_path(retval);
	return retval;
}

vector<string> ScriptParser::includePaths;

string& cleanInclude(string& includePath)
{
	//Add a `/` to the end of the include path, if it is missing
	int32_t lastnot = includePath.find_last_not_of("/\\");
	int32_t last = includePath.find_last_of("/\\");
	if(lastnot != string::npos)
	{
		if(last == string::npos || last < lastnot)
			includePath += "/";
	}
	regulate_path(includePath);
	return includePath;
}

string* ScriptParser::checkIncludes(string& includePath, string const& importname, vector<string> includes)
{
	for (size_t q = 0; q < includes.size(); ++q ) //Loop through all include paths, or until valid file is found
	{
		includePath = includes.at(q);
		cleanInclude(includePath);
		includePath = prepareFilename(includePath + importname);
		FILE* f = fopen(includePath.c_str(), "r");
		if(!f) continue;
		fclose(f);
		return &includePath;
	}
	return NULL;
}

bool ScriptParser::valid_include(ASTImportDecl& decl, string& ret_fname)
{
	if(decl.wasValidated())
	{
		ret_fname = decl.getFilename();
		return true;
	}
	string* fname = NULL;
	string includePath;
	string importname = prepareFilename(decl.getFilename());
	if(!decl.isInclude()) //Check root dir first for imports
	{
		FILE* f = fopen(importname.c_str(), "r");
		if(f)
		{
			fclose(f);
			fname = &importname;
		}
	}
	if(!fname)
	{
		// Scan include paths
		int32_t importfound = importname.find_first_not_of("/\\");
		if(importfound != string::npos) //If the import is not just `/`'s and `\`'s...
		{
			if(importfound != 0)
				importname = importname.substr(importfound); //Remove leading `/` and `\`
			//Convert the include string to a proper import path
			fname = checkIncludes(includePath, importname, ZQincludePaths);
			if(!fname)
			{
				fname = checkIncludes(includePath, importname, includePaths);
			}
		}
	}
	string filename = fname ? *fname : prepareFilename(importname); //Check root dir last, if nothing has been found yet.
	ret_fname = filename;
	FILE* f = fopen(filename.c_str(), "r");
	if(f)
	{
		fclose(f);
		//zconsole_db("Importing filename '%s' successfully", filename.c_str());
		decl.setFilename(filename);
		decl.validate();
		return true;
	}
	else return false;
}

bool ScriptParser::preprocess_one(ASTImportDecl& importDecl, int32_t reclimit)
{
	string filename;
	if(!valid_include(importDecl, filename))
	{
		log_error(CompileError::CantOpenImport(&importDecl, filename));
		return false;
	}
	unique_ptr<ASTFile> imported(parseFile(filename));
	if (!imported.get())
	{
		log_error(CompileError::CantParseImport(&importDecl, filename));
		return false;
	}

	// Save the AST in the import declaration.
	importDecl.giveTree(imported.release());

	// Recurse on imports.
	if (!preprocess(importDecl.getTree(), reclimit - 1))
		return false;

	return true;
}

bool ScriptParser::preprocess(ASTFile* root, int32_t reclimit)
{
	assert(root);
	
	if (reclimit == 0)
	{
		log_error(CompileError::ImportRecursion(NULL, recursionLimit));
		return false;
	}
	for(auto it = root->inclpaths.begin();
	     it != root->inclpaths.end(); ++it)
	{
		bool dupe = false;
		string& path = cleanInclude((*it)->path);
		for(auto it2 = includePaths.begin();
			it2 != includePaths.end(); ++it2)
		{
			if(!strcmp((*it2).c_str(), path.c_str()))
			{
				dupe = true;
				break;
			}
		}
		if(!dupe)
			includePaths.push_back(path);
	}
	// Repeat parsing process for each of import files
	bool ret = true;
	for (auto it = root->imports.begin();
	     it != root->imports.end(); ++it)
	{
		if(!preprocess_one(**it, reclimit)) ret = false;
	}

	return ret;
}

unique_ptr<IntermediateData> ScriptParser::generateOCode(FunctionData& fdata)
{
	Program& program = fdata.program;
	Scope* scope = &program.getScope();
	TypeStore* typeStore = &program.getTypeStore();
	vector<Datum*>& globalVariables = fdata.globalVariables;

	// Z_message("yes");
	bool failure = false;

	//we now have labels for the functions and ids for the global variables.
	//we can now generate the code to intialize the globals
	unique_ptr<IntermediateData> rval(new IntermediateData(fdata));

	// Push 0s for init stack space.
	/* Why? The stack should already be init'd to all 0, anyway?
	rval->globalsInit.push_back(
			new OSetImmediate(new VarArgument(EXP1),
			                  new LiteralArgument(0)));
	int32_t globalStackSize = *program.getScope().getRootStackSize();
	for (int32_t i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPushRegister(new VarArgument(EXP1)));*/

	// Generate variable init code.
	for (vector<Datum*>::iterator it = globalVariables.begin();
	     it != globalVariables.end(); ++it)
	{
		Datum& variable = **it;
		AST& node = *variable.getNode();

		OpcodeContext oc(typeStore);

		BuildOpcodes bo(scope);
		node.execute(bo, &oc);
		if (bo.hasError()) failure = true;
		appendElements(rval->globalsInit, oc.initCode);
		appendElements(rval->globalsInit, bo.getResult());
	}

	// Pop off everything.
	/* See above; why push this in the first place?
	for (int32_t i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPopRegister(new VarArgument(EXP2)));*/
	
	//Parse the indexes for class variables
	for(UserClass* user_class : program.classes)
	{
		user_class->getScope().parse_ucv();
	}
	
	//globals have been initialized, now we repeat for the functions
	vector<Function*> funs = program.getUserFunctions();
	appendElements(funs, program.getUserClassConstructors());
	appendElements(funs, program.getUserClassDestructors());
	for (vector<Function*>::iterator it = funs.begin();
	     it != funs.end(); ++it)
	{
		Function& function = **it;
		bool classfunc = function.getFlag(FUNCFLAG_CLASSFUNC) && !function.getFlag(FUNCFLAG_STATIC);
		int puc = 0;
		if(classfunc)
		{
			if(function.getFlag(FUNCFLAG_CONSTRUCTOR))
				puc = puc_construct;
			else if(function.getFlag(FUNCFLAG_DESTRUCTOR))
				puc = puc_destruct;
			else puc = puc_funcs;
		}
		if(function.getFlag(FUNCFLAG_INLINE)) continue; //Skip inline func decls, they are handled at call location -V
		if(puc != puc_construct && function.prototype) continue; //Skip prototype func decls, they are ALSO handled at the call location -V
		ASTFuncDecl& node = *function.node;

		bool isRun = ZScript::isRun(function);
		string scriptname;
		Script* functionScript = function.getScript();
		if (functionScript)
		{
			scriptname = functionScript->getName();
		}
		scope = function.internalScope;
		
		if(classfunc)
		{
			UserClass& user_class = scope->getClass()->user_class;
			
			vector<std::shared_ptr<Opcode>> funccode;
			
			int32_t stackSize = getStackSize(function);
			// Start of the function.
			if (puc == puc_construct)
			{
				vector<Function*> destr = user_class.getScope().getDestructor();
				std::shared_ptr<Opcode> first;
				Function* destructor = destr.size() == 1 ? destr.at(0) : nullptr;
				if(destructor && !destructor->prototype)
				{
					Function* destructor = destr[0];
					first.reset(new OSetImmediate(new VarArgument(EXP1),
						new LabelArgument(destructor->getLabel())));
				}
				else first.reset(new OSetImmediate(new VarArgument(EXP1),
					new LiteralArgument(0)));
				first->setLabel(function.getLabel());
				funccode.push_back(std::move(first));
				addOpcode2(funccode, new OConstructClass(new VarArgument(CLASS_THISKEY),
					new VectorArgument(user_class.members)));
				std::shared_ptr<Opcode> alt(new ONoOp());
				alt->setLabel(function.getAltLabel());
				funccode.push_back(std::move(alt));
			}
			else if(puc == puc_destruct)
			{
				std::shared_ptr<Opcode> first(new ODestructor(new StringArgument(user_class.getName())));
				first->setLabel(function.getLabel());
				funccode.push_back(std::move(first));
			}
			else
			{
				std::shared_ptr<Opcode> first(new OSetImmediate(new VarArgument(EXP1),
					new LiteralArgument(0)));
				first->setLabel(function.getLabel());
				funccode.push_back(std::move(first));
			}
			// Push 0s for the local variables.
			for (int32_t i = stackSize - getParameterCount(function); i > 0; --i)
				addOpcode2(funccode, new OPushImmediate(new LiteralArgument(0)));
			
			// Set up the stack frame register
			addOpcode2(funccode, new OSetRegister(new VarArgument(SFRAME),
												new VarArgument(SP)));
			OpcodeContext oc(typeStore);
			BuildOpcodes bo(scope);
			bo.parsing_user_class = puc;
			node.execute(bo, &oc);
			
			if (bo.hasError()) failure = true;
			
			appendElements(funccode, bo.getResult());
			
			// Pop off everything
			std::shared_ptr<Opcode> next(new OPopArgsRegister(new VarArgument(NUL),
				new LiteralArgument(stackSize)));
			next->setLabel(bo.getReturnLabelID());
			funccode.push_back(std::move(next));
			if (puc == puc_construct) //return val
				addOpcode2(funccode, new OSetRegister(new VarArgument(EXP1), new VarArgument(CLASS_THISKEY)));
			addOpcode2(funccode, new OReturn());
			function.giveCode(funccode);
		}
		else
		{
			vector<std::shared_ptr<Opcode>> funccode;
			
			int32_t stackSize = getStackSize(function);
			
			// Start of the function.
			std::shared_ptr<Opcode> first(new OSetImmediate(new VarArgument(EXP1),
											  new LiteralArgument(0)));
			first->setLabel(function.getLabel());
			funccode.push_back(std::move(first));
			
			// Push on the this, if a script
			if (isRun)
			{
				ScriptType type = program.getScript(scriptname)->getType();

				if (type == ScriptType::ffc )
				{
					addOpcode2(funccode, 
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFFFC)));


				}
				else if (type == ScriptType::item )
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFITEMCLASS)));

				}
				else if (type == ScriptType::npc )
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFNPC)));

				}
				else if (type == ScriptType::lweapon )
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFLWPN)));
				}
				else if (type == ScriptType::eweapon )
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFEWPN)));

				}
				else if (type == ScriptType::dmapdata )
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								 new VarArgument(REFDMAPDATA)));

				}
				else if (type == ScriptType::itemsprite)
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								new VarArgument(REFITEM)));
				}
				else if (type == ScriptType::subscreendata)
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								new VarArgument(REFSUBSCREEN)));
				}
				else if (type == ScriptType::combodata)
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								new VarArgument(REFCOMBODATA)));
				}
				else if (type == ScriptType::genericscr)
				{
					addOpcode2(funccode,
						new OSetRegister(new VarArgument(EXP2),
								new VarArgument(REFGENERICDATA)));
				}
				
				addOpcode2(funccode, new OPushRegister(new VarArgument(EXP2)));
			}
			
			// Push 0s for the local variables.
			for (int32_t i = stackSize - getParameterCount(function); i > 0; --i)
				addOpcode2(funccode, new OPushRegister(new VarArgument(EXP1)));
			
			// Set up the stack frame register
			addOpcode2(funccode, new OSetRegister(new VarArgument(SFRAME),
												new VarArgument(SP)));
			OpcodeContext oc(typeStore);
			BuildOpcodes bo(scope);
			node.execute(bo, &oc);
			
			if (bo.hasError()) failure = true;
			
			appendElements(funccode, bo.getResult());
			
			// Add appendix code.
			std::shared_ptr<Opcode> next(new OSetImmediate(new VarArgument(EXP2),
													  new LiteralArgument(0)));
			next->setLabel(bo.getReturnLabelID());
			funccode.push_back(std::move(next));
			
			// Pop off everything.
			for (int32_t i = 0; i < stackSize; ++i)
			{
				addOpcode2(funccode, new OPopRegister(new VarArgument(EXP2)));
			}
			
			//if it's a main script, quit.
			if (isRun)
			{
				// Note: the stack still contains the "this" pointer
				// But since the script is about to terminate, we don't
				// care about popping it off.
				addOpcode2(funccode, new OQuit());
			}
			else
			{
				// Not a script's run method, so no "this" pointer to
				// pop off. The top of the stack is now the function
				// return address (pushed on by the caller).
				//pop off the return address
				//and return
				addOpcode2(funccode, new OReturn());
			}
			
			function.giveCode(funccode);
		}
	}

	if (failure)
	{
		rval.reset();
		return unique_ptr<IntermediateData>(rval.release());;
	}

	//Z_message("yes");
	return unique_ptr<IntermediateData>(rval.release());
}

static vector<shared_ptr<Opcode>> blankScript()
{
	vector<shared_ptr<Opcode>> rval;
	addOpcode2(rval, new OQuit());
	return rval;
}

void ScriptParser::assemble(IntermediateData *id)
{
	Program& program = id->program;

	map<Script*, vector<shared_ptr<Opcode>> > scriptCode;
	vector<shared_ptr<Opcode>> ginit = id->globalsInit;

	// Do the global inits

	// If there's a global script called "Init", append it to ~Init:
	Script* userInit = program.getScript("Init");
	if (userInit && userInit->getType() == ScriptType::global
		&& !userInit->isPrototypeRun()) //Prototype run function can be ignored, as it is empty.
	{
		int32_t label = *getLabel(*userInit);
		addOpcode2(ginit, new OGotoImmediate(new LabelArgument(label)));
	}

	Script* init = program.getScript("~Init");
	init->code = assembleOne(program, ginit, 0);

	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		if (script.getName() == "~Init") continue;
		if(script.getType() == ScriptType::untyped) continue;
		Function& run = *script.getRun();
		if(run.prototype) //Generate a minimal script if 'run()' is a prototype.
		{
			script.code = blankScript();
		}
		else
		{
			int32_t numparams = script.getRun()->paramTypes.size();
			script.code = assembleOne(program, run.getCode(), numparams);
		}
	}
}

vector<shared_ptr<Opcode>> ScriptParser::assembleOne(
		Program& program, vector<shared_ptr<Opcode>> runCode, int32_t numparams)
{
	std::vector<std::shared_ptr<Opcode>> rval;

	// Push on the params to the run.
	int32_t i;
	for (i = 0; i < numparams && i < 9; ++i)
		addOpcode2(rval, new OPushRegister(new VarArgument(i)));
	for (; i < numparams; ++i)
		addOpcode2(rval, new OPushRegister(new VarArgument(EXP1)));

	// Generate a map of labels to functions.
	vector<Function*> allFunctions = getFunctions(program);
	appendElements(allFunctions, program.getUserClassConstructors());
	appendElements(allFunctions, program.getUserClassDestructors());
	map<int32_t, Function*> functionsByLabel;
	for (vector<Function*>::iterator it = allFunctions.begin();
	     it != allFunctions.end(); ++it)
	{
		Function& function = **it;
		functionsByLabel[function.getLabel()] = &function;
		if(function.getFlag(FUNCFLAG_CONSTRUCTOR))
			functionsByLabel[function.getAltLabel()] = &function;
	}

	// Grab all labels directly jumped to.
	std::set<int32_t> usedLabels;
	for (vector<shared_ptr<Opcode>>::iterator it = runCode.begin();
	     it != runCode.end(); ++it)
	{
		GetLabels temp(usedLabels);
		(*it)->execute(temp, NULL);
	}
	std::set<int32_t> unprocessedLabels(usedLabels);

	// Grab labels used by each function until we run out of functions.
	while (!unprocessedLabels.empty())
	{
		int32_t label = *unprocessedLabels.begin();
		Function* function =
			find<Function*>(functionsByLabel, label).value_or(boost::add_pointer<Function>::type());
		if (function)
		{
			vector<shared_ptr<Opcode>> const& functionCode = function->getCode();
			for (vector<shared_ptr<Opcode>>::const_iterator it = functionCode.begin();
			     it != functionCode.end(); ++it)
			{
				GetLabels temp(usedLabels);
				(*it)->execute(temp, NULL);
				insertElements(unprocessedLabels, temp.newLabels);
			}
		}

		unprocessedLabels.erase(label);
	}

	// Make the rval
	for (vector<shared_ptr<Opcode>>::iterator it = runCode.begin();
	     it != runCode.end(); ++it)
		addOpcode2(rval, (*it)->makeClone());

	for (std::set<int32_t>::iterator it = usedLabels.begin();
	     it != usedLabels.end(); ++it)
	{
		int32_t label = *it;
		Function* function =
			find<Function*>(functionsByLabel, label).value_or(boost::add_pointer<Function>::type());
		if (!function) continue;

		vector<shared_ptr<Opcode>> functionCode = function->getCode();
		for (vector<shared_ptr<Opcode>>::iterator it = functionCode.begin();
		     it != functionCode.end(); ++it)
			addOpcode2(rval, (*it)->makeClone());
	}

	// Set the label line numbers.
	map<int32_t, int32_t> linenos;
	int32_t lineno = 1;

	for (vector<shared_ptr<Opcode>>::iterator it = rval.begin();
	     it != rval.end(); ++it)
	{
		if ((*it)->getLabel() != -1)
			linenos[(*it)->getLabel()] = lineno;
		lineno++;
	}

	// Now fill in those labels
	for (vector<shared_ptr<Opcode>>::iterator it = rval.begin();
	     it != rval.end(); ++it)
	{
		SetLabels temp;
		(*it)->execute(temp, &linenos);
	}

	return rval;
}

std::pair<int32_t,bool> ScriptParser::parseLong(std::pair<string, string> parts, Scope* scope)
{
	// Not sure if this should really check for negative numbers;
	// in most contexts, that's checked beforehand. parts only
	// includes the minus if this is a constant. - Saf
	bool negative=false;
	std::pair<int32_t, bool> rval;
	rval.second=true;
	bool intOneLarger = *lookupOption(*scope, CompileOption::OPT_TRUE_INT_SIZE) != 0;

	if(parts.first.data()[0]=='-')
	{
		negative=true;
		parts.first = parts.first.substr(1);
	}

	if(parts.second.size() > 4)
	{
		rval.second = false;
		parts.second = parts.second.substr(0,4);
	}

	if(parts.first.size() > 6)
	{
		rval.second = false;
		parts.first = parts.first.substr(0,6);
	}

	int32_t firstpart = atoi(parts.first.c_str());
	if(intOneLarger) //MAX_INT should be 214748, but if that is the value, there should be no float component. -V
	{
		if(firstpart > 214748)
		{
			firstpart = 214748;
			rval.second = false;
		}
	}
	else if(firstpart > 214747)
	{
		firstpart = 214747;
		rval.second = false;
	}

	int32_t intval = ((int32_t)(firstpart))*10000;
	//add fractional part; tricky!
	int32_t fpart = 0;


	while(parts.second.length() < 4)
		parts.second += "0";

	for(uint32_t i = 0; i < 4; i++)
	{
		fpart *= 10;
		fpart += parts.second[i] - '0';
	}

	/*for(uint32_t i=0; i<4; i++)
	  {
	  fpart*=10;
	  char tmp[2];
	  tmp[0] = parts.second.at(i);
	  tmp[1] = 0;
	  fpart += atoi(tmp);
	  }*/

	if(intOneLarger && firstpart == 214748 && (negative ? fpart > 3648 : fpart > 3647))
	{
		fpart = negative ? 3648 : 3647;
		rval.second = false;
	}


	rval.first = intval + fpart;
	if(negative)
		rval.first = -rval.first;
	return rval;
}

ScriptsData::ScriptsData(Program& program)
{
	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		string const& name = script.getName();
		zasm_meta& meta = theScripts[name].first;
		theScripts[name].second = script.code;
		meta = script.getMetadata();
		meta.script_type = script.getType().getTrueId();
		meta.script_name = name;
		meta.author = script.getAuthor();
		if(Function* run = script.getRun())
		{
			int32_t ind = 0;
			for(vector<string const*>::const_iterator it = run->paramNames.begin();
				it != run->paramNames.end(); ++it)
			{
				meta.run_idens[ind] = (**it);
				if(!meta.initd[ind].size())
					meta.initd[ind] = meta.run_idens[ind];
				++ind;
			}
			ind = 0;
			for(vector<DataType const*>::const_iterator it = run->paramTypes.begin();
				it != run->paramTypes.end(); ++it)
			{
				std::optional<DataTypeId> id = program.getTypeStore().getTypeId(**it);
				meta.run_types[ind] = id ? *id : ZVARTYPEID_VOID;
				int8_t ty = -1;
				if(id) switch(*id)
				{
					case ZVARTYPEID_BOOL:
						ty = nswapBOOL;
						break;
					case ZVARTYPEID_LONG:
						ty = nswapLDEC;
						break;
					case ZVARTYPEID_FLOAT:
					case ZVARTYPEID_UNTYPED:
						ty = nswapDEC;
						break;
				}
				if(meta.initd_type[ind] < 0)
					meta.initd_type[ind] = ty;
				++ind;
			}
		}

		script.code = vector<shared_ptr<Opcode>>();
		scriptTypes[name] = script.getType();
	}
}