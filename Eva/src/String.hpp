#pragma once
#include "object.h"
#include <ostream>
class String  : public Object
{
public:
	String(const char* str, size_t size);
	String(const String& string);
	String(String&& string);
	~String();
	
private:
	friend std::ostream& operator<<(std::ostream& os, String& string);
	int m_Size;
	char* m_Str;
};
inline std::ostream& operator<<(std::ostream& os, String& string)
{
	os << string.m_Str;
	return os;
}