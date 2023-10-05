#include "String.hpp"
#include <cstring>
#include <iostream>

String::String(const char* str, size_t size) : m_Size(size+1)
{
	m_Str = new char[m_Size];
	memcpy(m_Str, str, m_Size * sizeof(char));
	m_Str[m_Size - 1] = '\0';
}
String::String(const String& string) 
{
	std::cout << "string copy\n";
	m_Size = string.m_Size;
	m_Str = new char[string.m_Size];
	memcpy(m_Str, string.m_Str, m_Size * sizeof(char));
	m_Str[m_Size - 1] = '\0';
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

bool String::operator==(const String& str)
{
	if (str.m_Size != m_Size) return false;
	return !static_cast<bool>(strcmp(m_Str,str.m_Str));
}

String& String::operator=(const String& string)
{
	if (this != &string)
	{
		std::cout << "string copy\n";
		m_Size = string.m_Size;
		m_Str = new char[string.m_Size];
		memcpy(m_Str, string.m_Str, m_Size * sizeof(char));
		m_Str[m_Size - 1] = '\0';
	}
	
	return *this;
}

