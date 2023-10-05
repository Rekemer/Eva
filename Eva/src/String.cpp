#include "String.hpp"
#include <cstring>

String::String(const char* str, size_t size) : m_Size(size+1)
{
	m_Str = new char[m_Size];
	strcpy_s(m_Str, m_Size,str);
}
String::String(const String& string) 
{
	m_Size = string.m_Size;
	m_Str = new char[string.m_Size];
	strcpy_s(m_Str, string.m_Size, string.m_Str);
}
String::String(String&& string)
{
	m_Str = string.m_Str;
	m_Size = string.m_Size;

	delete[] string.m_Str;
	string.m_Size = 0;
}

String::~String()
{
	delete[] m_Str;
}