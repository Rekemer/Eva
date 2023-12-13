#include "String.hpp"
#include <cstring>
#include <iostream>

String::String(const char* str, size_t size) : m_Size(size+1)
{
	//std::cout << "string alloc\n";
	m_Data = new char[m_Size];
	memcpy(m_Data, str, m_Size * sizeof(char));
	m_Data[m_Size - 1] = '\0';
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
//	m_Data = new char[m_Size];
//	memcpy(m_Data, str, m_Size * sizeof(char));
//	m_Data[m_Size - 1] = '\0';
//}

String::String(const String& string) 
{
	
	//std::cout << "string copy\n";
	m_Size = string.m_Size;
	m_Data = new char[string.m_Size];
	memcpy(m_Data, string.m_Data, m_Size * sizeof(char));
	m_Data[m_Size - 1] = '\0';
}
String::String(String&& string)
{
	m_Data = string.m_Data;
	m_Size = string.m_Size;

	delete[] string.m_Data;
	string.m_Size = 0;
}

String::~String()
{
	delete[] m_Data;
}

bool String::operator==(const String& str)
{
	return AreEqual(m_Data, GetSize(), str.GetRaw(), str.GetSize());
}
bool String::operator==(const char* str)
{
	return strcmp(m_Data,str) == 0;
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
		m_Data = new char[string.m_Size];
		memcpy(m_Data, string.m_Data, m_Size * sizeof(char));
		m_Data[m_Size - 1] = '\0';
	}
	
	return *this;
}

