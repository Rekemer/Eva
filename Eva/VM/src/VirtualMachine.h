#pragma once
#include <vector>
#include <stack>
#include <array>
#include <memory>
#include <cstdlib>
#include "Value.h"
#include "Function.h"
#include "HashTable.h"
#include "Bytecode.h"
#include "CallFrame.h"
#include "PluginData.h"

 


namespace Eva
{

	class AST;
	struct Node;
	struct Block;
	struct Expression;
	struct Scope;
	struct StackSim;
	struct Operand;
	struct Instruction;
	class CFG;
	class VirtualMachine
	{
	public:
		VirtualMachine() = default;
		VirtualMachine(const VirtualMachine& vm)
		{
			internalStrings = vm.internalStrings;
			globalVariables = vm.globalVariables;
			globalVariablesTypes = vm.globalVariablesTypes;
			vmStack = vm.vmStack;

		}
		void Execute();
		const std::vector<ValueContainer>& GetStack() { return vmStack; };

		void DumpGlobalToFile(std::string_view filepath);
		HashTable& GetGlobals() { return globalVariables; };
		HashTable& GetGlobalsType() { return globalVariablesTypes; };
		ValueType GetGlobalType(const std::string& str);
		~VirtualMachine();
	private:
		void CastWithDeclared(ValueType assignedType, ValueType declared);
		void BeginContinue(int startLoopIndex);
		int BeginBreak();
		void EndContinue();
		void PatchBreak(int prevSizeBreak);
		bool AreEqual(const ValueContainer& a, const ValueContainer& b);

	public:
		// to execute global code
		std::unique_ptr<Func> globalFunc = std::make_unique<Func>();
		bool isTest = false;
		Func* mainFunc = nullptr;
		std::vector<std::string> functionNames;
		PluginTable pluginTable;
		std::vector<ValueContainer> vmStack;
	private:
		friend Func;
		friend NativeFunc;
		// function we build or execute
		Func* currentFunc = nullptr;
		// entry point



		HashTable internalStrings;
		HashTable globalVariables;
		HashTable globalVariablesTypes;

		ValueContainer temp;

		std::array<CallFrame, callFrameAmount > callFrames;
		
		bool m_Panic = false;
		// if we hit break or continue we should know where to jump
		// we need stack because there can be loops in loops
		std::stack<int> m_StartLoopIndexes;
		// Once we hit break we should remember the index
		// to come back and  patch it with correct distance
		// for a jump
		std::stack<int> m_BreakIndexes;
		std::stack<int> conditionIndex;
		ValueType m_FuncReturnType = ValueType::NIL;
		std::stack<ValueType> lastReturnType;
	public:
		Scope* currentScope = nullptr;
	};
}