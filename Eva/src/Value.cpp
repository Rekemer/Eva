#include"Value.h"
#include<cassert>


const char* ValueToStr(ValueType valueType) {
switch (valueType)
{
	case ValueType::FLOAT:
		return "FLOAT";
	case ValueType::INT:
		return "INT";
	case ValueType::BOOL:
		return "BOOL";
	case ValueType::STRING:
		return "STRING";
	case ValueType::NIL:
		return "NIL";
	case ValueType::FUNCTION:
		return "FUNCTION";
	default:
		return "Unknown value type";
	}
}