#include "VirtualMachine.h"
#include "Incode.h"
#include "Debug.h"
#include <string>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <format>
#include "TokenConversion.h"
#include "Local.h"
#include "ICallable.h"
#include "Native.h"


#include <fstream>
#include "Serialize.h"
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.back().As<type>();\
vmStack.pop_back();\
auto v2 = vmStack.back().As<type>();\
vmStack.pop_back();\
vmStack.push_back(ValueContainer{v2 operation v});\
}\

#define UNARY_OP(type,operation)\
{\
auto v = vmStack.back().as.type;\
vmStack.pop_back();\
auto v2 = vmStack.back().as.type;\
vmStack.pop_back();\
vmStack.push_back(ValueContainer{v2 operation v});\
}\
while(false)



ValueType DetermineOpTypeRet(ValueType type, InCode op, Func * currentFunc)
{
	assert(type != ValueType::NIL);
	switch (op)
	{
	case InCode::MULTIPLY:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::MULTIPLY_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::MULTIPLY_FLOAT);
			return ValueType::FLOAT;
		}
		break;

	case InCode::SUBSTRACT:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::SUBSTRACT_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::SUBSTRACT_FLOAT);
			return ValueType::FLOAT;
		}
		break;

	case InCode::ADD:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::ADD_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::ADD_FLOAT);
			return ValueType::FLOAT;
		}
		break;
	case InCode::DIVIDE:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_FLOAT);
			return ValueType::FLOAT;
		}
		break;
	case InCode::DIVIDE_PERCENT:
		{
			currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_PERCENT);
			return ValueType::INT;
			break;
		}
	default:
		assert(false && "Unknown operation");
		return ValueType::NIL;
	}

	// If none of the conditions match, return NIL as a fallback.
	return ValueType::NIL;
}


// deterni
#define DETERMINE_OP_TYPE(type,OP)\
{\
	if (type == ValueType::INT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else if (type == ValueType::FLOAT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
	else{assert(false && "unknown type of operation");}\
}\

#define DETERMINE_BOOL(left,right,OP)\
{\
	if (left == ValueType::INT && right == ValueType::INT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
}\

#define CAST_BOOL(type)\
if (type== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_BOOL_INT));\
}else if (type== ValueType::FLOAT)\
{\
currentFunc->opCode.push_back(((uint8_t)InCode::CAST_BOOL_FLOAT));\
}\

#define CAST_INT_FLOAT(type1,type2)\
if (type1== ValueType::INT && type2== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_FLOAT));\
}\
else if (type1== ValueType::FLOAT && type2== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_INT));\
}\


void VirtualMachine::CastWithDeclared(ValueType assignedType, ValueType declared)
{
	if (assignedType == declared) return;
	assert(assignedType == ValueType::INT || assignedType == ValueType::FLOAT);
	assert(declared == ValueType::INT || declared == ValueType::FLOAT);
	if (assignedType != declared && assignedType != ValueType::NIL)
	{
		if (declared == ValueType::INT)
		{
			currentFunc->opCode.push_back(((Bytecode)InCode::CAST_INT));
		}
		else
		{
			currentFunc->opCode.push_back(((Bytecode)InCode::CAST_FLOAT));
		}
	}
}


#define CAST_INT(child)\
if (child== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_INT));\
}\

// returns the index for backpatching
int JumpIfFalse(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_IF_FALSE);
	// the address of else byteblock code - backpatching
	// then branch
	opCode.push_back((uint8_t)0);
	auto indexJumpFalse = opCode.size() - 1;
	return indexJumpFalse;
}
int Jump(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
int JumpBack(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_BACK);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
// from: to make jump relative
int CalculateJumpIndex(const std::vector<Bytecode> const &  opCode, const int from)
{
	return opCode.size() - 1 - from;
}
void EmitPop(std::vector<Bytecode>& opCode)
{
	opCode.push_back((Bytecode)InCode::POP);
}

ValueType VirtualMachine::GetGlobalType(const std::string& str)
{
	auto entry = globalVariablesTypes.Get(str);
	if (!entry->IsInit())
	{
		//std::stringstream ss;
		//ss << "ERROR[" << (expr->line) << "]: " <<
		//	"The name " << str << " is used but not declared " << std::endl;
		auto mes = std::format("The name {} is used but not declared ", str);
		throw std::exception{ mes.c_str() };
	}
	return entry->value.type;
}

void VirtualMachine::BeginContinue(int startLoopIndex)
{
	m_StartLoopIndexes.push(startLoopIndex);
}
void VirtualMachine::EndContinue()
{
	m_StartLoopIndexes.pop();
}
int VirtualMachine::BeginBreak()
{
	return m_BreakIndexes.size();
}
void VirtualMachine::PatchBreak(int prevSizeBreak)
{
	// we hit break we should patch it
	if (m_BreakIndexes.size() - prevSizeBreak > 0)
	{
		// we need to skip one pop because there is condition check on the stack 
		// when we execute body of the loop, hence +2. 
		// 1 to get to the future instruction
		// 1 to skip pop opeation
		auto index = m_BreakIndexes.top();
		currentFunc->opCode[index] =
			CalculateJumpIndex(currentFunc->opCode, index) + 2;
		m_BreakIndexes.pop();
	}
}
void VirtualMachine::GenerateConstant(const ValueContainer& v)
{
	if (v.type == ValueType::BOOL)
	{
		auto instr = v.As<ebool>() ? (Bytecode)InCode::TRUE : (Bytecode)InCode::FALSE;
		currentFunc->opCode.push_back(instr);
		return;
	}
	currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
	currentFunc->constants.push_back(v);
	currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
}

VirtualMachine::~VirtualMachine()
{
}


bool VirtualMachine::AreEqual(const ValueContainer& a, const ValueContainer& b)
{
	if (a.type == b.type && a.type == ValueType::BOOL)
	{
		return a.As<ebool>() == b.As<ebool>();
	}
	else if (a.type == b.type && a.type == ValueType::FLOAT)
	{
		return fabs(a.As<efloat>() - b.As<efloat>()) < 0.04;
	}
	else if (a.type == b.type && a.type == ValueType::INT)
	{
		return a.As<eint>() == b.As<eint>();
	}
	else if (a.type == b.type && a.type == ValueType::STRING)
	{
		auto str = a.AsString();
		auto str2 = b.AsString();
		return str == str2;
	}
	assert(false && "cannot compare the values of different type");
	return false;
}

void VirtualMachine::DumpGlobalToFile(std::string_view filepath)
{
	std::ofstream os(filepath.data());
	cereal::JSONOutputArchive archive(os);
	archive(globalVariables);
}

void VirtualMachine::Execute()
{
	if (m_Panic)return;
	globalFunc->opCode.push_back((uint8_t)InCode::NIL);
	globalFunc->opCode.push_back((uint8_t)InCode::RETURN);
	if (mainFunc)
	{
		callFrames[nextToCurrentCallFrame++].function = mainFunc;
	}
	callFrames[nextToCurrentCallFrame].function = globalFunc.get();
	callFrames[nextToCurrentCallFrame].stackIndex = -1;
	nextToCurrentCallFrame++;
	auto frame = &callFrames[nextToCurrentCallFrame-1];
	while (true)
	{
		// check if there are strings to be freed
		// it doesn't have to be here, only for debugging
		// usually memory is freed when we want to allocate new memory
		//CollectStrings();
		auto inst = frame->function->opCode[frame->ip++];
		//std::cout << std::format("instr index: {}\n",frame->ip - 1);
		//globalVariables.Print();
		switch (static_cast<InCode>(inst))
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push_back(frame->function->constants[frame->function->opCode[frame->ip++]]);
			break;
		}
		case InCode::TRUE:
		{
			vmStack.push_back(ValueContainer{ true });
			break;
		}case InCode::NIL:
		{
			vmStack.push_back(ValueContainer{ -1 });
			break;
		}
		case InCode::FALSE:
		{
			vmStack.push_back(ValueContainer{ false });
			break;
		}
		case InCode::ADD_FLOAT:
		{
			BINARY_OP(efloat, +);
			break;
		}
		case InCode::SUBSTRACT_FLOAT:
		{
			BINARY_OP(efloat, -);
			break;
		}
		case InCode::MULTIPLY_FLOAT:
		{
			BINARY_OP(efloat, *);
			break;
		}
		case InCode::DIVIDE_FLOAT:
		{
			BINARY_OP(efloat, / );
			break;
		}
		case InCode::ADD_INT:
		{
			BINARY_OP(eint, +);
			break;
		}
		case InCode::ADD_STRING:
		{
			auto v = vmStack.back().AsString();
			vmStack.pop_back();
			auto v2 = vmStack.back().AsString();
			vmStack.pop_back();
			//auto newString = VirtualMachine::AddStrings(v2,v);
			auto newString = v2 + v;
			vmStack.push_back(ValueContainer{ newString }); \

				break;
		}
		case InCode::CAST_FLOAT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::FLOAT;
			float v = value.As<eint>();
			value.as = v;
			break;
		}
		case InCode::CAST_INT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::INT;
			int v = value.As<efloat>();
			value.as = v;
			break;
		}
		case InCode::CAST_BOOL_INT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::BOOL;
			bool v = value.As<eint>();
			value.as = v;
			break;
		}
		case InCode::CAST_BOOL_FLOAT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::BOOL;
			bool v = value.As<efloat>();
			value.as = v;
			break;
		}
		case InCode::SUBSTRACT_INT:
		{
			BINARY_OP(eint, -);
			break;
		}
		case InCode::MULTIPLY_INT:
		{
			BINARY_OP(eint, *);
			break;
		}
		case InCode::DIVIDE_INT:
		{
			BINARY_OP(eint, / );
			break;
		}
		case InCode::DIVIDE_PERCENT:
		{
			BINARY_OP(eint, %);
			break;
		}
		case InCode::INCREMENT_INT:
		{
			auto& value = vmStack.back();
			value.AsRef<eint>()++;
			break;
		}
		case InCode::DECREMENT_INT:
		{
			auto& value = vmStack.back();
			value.AsRef<eint>()--;
			break;
		}
		case InCode::INCREMENT_FLOAT:
		{
			auto& value = vmStack.back();
			value.AsRef<float>()++;
			break;
		}
		case InCode::DECREMENT_FLOAT:
		{
			auto& value = vmStack.back();
			value.AsRef<float>()--;
			break;
		}
		case InCode::NEGATE:
		{
			auto value = vmStack.back();
			vmStack.pop_back();
			if (value.type == ValueType::FLOAT)
			{
				vmStack.push_back(ValueContainer{ -value.As<efloat>() });
			}
			else if (value.type == ValueType::INT)
			{
				vmStack.push_back(ValueContainer{ -value.As<eint>() });
			}
			else
			{
				assert("Unknown type to negate" && false);
			}
			break;
		}
		case InCode::NOT:
		{
			auto value = vmStack.back();
			vmStack.pop_back();
			vmStack.push_back(ValueContainer{ !value.As<ebool>() });
			break;
		}
		case InCode::LESS_FLOAT:
		{
			BINARY_OP(efloat, < );
			break;
		}
		case InCode::GREATER_FLOAT:
		{
			BINARY_OP(efloat, > );
			break;
		}
		case InCode::LESS_INT:
		{
			BINARY_OP(eint, < );
			break;
		}
		case InCode::GREATER_INT:
		{
			BINARY_OP(eint, > );
			break;
		}

		case InCode::EQUAL_EQUAL:
		{
			auto  v2 = vmStack.back();
			vmStack.pop_back();
			auto  v1 = vmStack.back();
			vmStack.pop_back();
			vmStack.push_back(ValueContainer{ AreEqual(v1,v2) });
			break;
		}
		case InCode::AND:
		{
			BINARY_OP(ebool, &&);
			break;
		}
		case InCode::OR:
		{
			BINARY_OP(ebool, || );
			break;
		}
		case InCode::RETURN:
		{

			//if (globalFunc.get() == callFrames[nextToCurrentCallFrame - 1].function)
			//{
			//	return;
			//}
			auto prevCallFrameIndex = nextToCurrentCallFrame - 2;
			auto res = vmStack.back();
			if (prevCallFrameIndex < 0)
			{
				return;
			}
			if (callFrames[nextToCurrentCallFrame - 1].function == globalFunc.get())
			{
				vmStack.resize(0);
			}
			else
				vmStack.resize(callFrames[nextToCurrentCallFrame - 1].stackIndex);
			vmStack.push_back(res);
			nextToCurrentCallFrame--;
			frame = &callFrames[prevCallFrameIndex];
			break;
		}
		case InCode::GET_GLOBAL_VAR:
		{
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = nameOfVariable.AsString();
			auto entry = globalVariables.Get(string);
			vmStack.push_back(entry->value);
			break;
		}
		case InCode::SET_GLOBAL_VAR:
		{
			auto& value = vmStack.back();
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = nameOfVariable.AsString();
			auto entry = globalVariables.Get(string);
			entry->value = value;
			vmStack.pop_back();
			break;
		}

		case InCode::GET_LOCAL_VAR:
		{
			auto index = frame->function->opCode[frame->ip++] + 1;
			vmStack.push_back(vmStack[frame->stackIndex + index]);
			break;
		}
		case InCode::SET_LOCAL_VAR:
		{
			auto index = frame->function->opCode[frame->ip++];
			auto value = vmStack.back();
			vmStack[index] = value;
			vmStack.pop_back();
			break;
		}

		case InCode::JUMP_IF_FALSE:
		{
			// if it is not false, then we should get to then block
			auto offset = frame->function->opCode[frame->ip++];
			auto condition = vmStack.back().As<ebool>();
			if (!condition) frame->ip = (frame->ip - 1) + (offset);
			break;
		}
		case InCode::JUMP:
		{
			auto offset = frame->function->opCode[frame->ip];
			frame->ip += offset;
			break;
		}
		case InCode::JUMP_BACK:
		{
			auto offset = frame->function->opCode[frame->ip];
			frame->ip -= offset;
			break;
		}
		case InCode::GET_NATIVE_NAME:
		{
			auto index = frame->function->opCode[frame->ip++];
			auto name = frame->function->constants[index];
			auto v = ValueContainer{ GetNative(name.AsString()) };
			vmStack.push_back(v);
			break;
		}
		case InCode::CALL:
		{
			// up to this point arguments and function are on stack
			//auto argumentCount = frame->function->opCode[frame->ip++];
			//auto funcIndex = vmStack.size()  - 1 - argumentCount;
			//auto func = vmStack[funcIndex].AsFunc();
			//auto newIndexFrame = CallFunction(func.get(), argumentCount, funcIndex);
			//// update our call frame 
			//frame = &callFrames[newIndexFrame];

			// arguments and function are on stack
			auto argumentCount = frame->function->opCode[frame->ip++];
			auto funcIndex = vmStack.size() - 1 - argumentCount;

			// Retrieve the ICallable from the stack
			auto callable = vmStack[funcIndex].AsCallable();

			// "Call" the function
			size_t newFrameIndex = callable->Call(*this, argumentCount, funcIndex);

			// If it's a user function, newFrameIndex is a valid frame
			// If it's a native function, newFrameIndex might be SIZE_MAX
			if (newFrameIndex != SIZE_MAX) {
				// Switch to that new frame
				frame = &callFrames[newFrameIndex];
			}
			else {
				// It's a native function, we stay in the current frame
				// Possibly pop the function and its arguments from the stack
				// if that's your calling convention
			}


			break;
		}
		case InCode::POP:
		{
			vmStack.pop_back();
			break;
		}
		case InCode::STORE_TEMP:
		{
			temp = vmStack.back();
			break;
		}
		case InCode::LOAD_TEMP:
		{
			vmStack.push_back(temp);
			break;
		}
		default:
			break;
		}
	}

	
}

size_t VirtualMachine::CallFunction(Func* func, size_t argumentCount,size_t baseIndex)
{
	auto& frame = callFrames[nextToCurrentCallFrame++];
	frame.function = func;
	frame.ip = 0;
	frame.stackIndex = baseIndex;
	return nextToCurrentCallFrame - 1;

}



