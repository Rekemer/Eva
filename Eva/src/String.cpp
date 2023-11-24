#include "String.hpp"
#include <cstring>
#include <iostream>

String::String(const char* str, size_t size) : m_Size(size+1)
{
	//std::cout << "string alloc\n";
	m_Str = new char[m_Size];
	memcpy(m_Str, str, m_Size * sizeof(char));
	m_Str[m_Size - 1] = '\0';
}
//String::String(const char* str)
//{
//	std::cout << "string alloc\n";
//	auto iter = str;
//	while (*iter != '\0')
//	{
//		iter++;
//	}
//	auto size = iter - str;
//	m_Size = size + 1;
//	m_Str = new char[m_Size];
//	memcpy(m_Str, str, m_Size * sizeof(char));
//	m_Str[m_Size - 1] = '\0';
//}

String::String(const String& string) 
{
	
	//std::cout << "string copy\n";
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
	return AreEqual(m_Str, GetSize(), str.GetRaw(), str.GetSize());
}

bool String::AreEqual(const char* str, size_t size, const char* str2, size_t size2)
{
	if (size != size2) return false;
	return !static_cast<bool>(strncmp(str, str2,size));
}

String& String::operator=(const String& string)
{
	if (this != &string)
	{
		//std::cout << "string copy\n";
		m_Size = string.m_Size;
		m_Str = new char[string.m_Size];
		memcpy(m_Str, string.m_Str, m_Size * sizeof(char));
		m_Str[m_Size - 1] = '\0';
	}
	
	return *this;
}

