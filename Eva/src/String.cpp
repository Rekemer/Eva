#include "String.hpp"
#include <cstring>
#include <iostream>

String::String(const char* str, size_t size) : m_Size(size + 1)
{
	m_Data = std::make_unique<char[]>(m_Size);
	std::copy(str,str+m_Size, m_Data.get());
	m_Data[m_Size - 1] = '\0';
}

String::String(const char* str)
{
	std::cout << "string alloc\n";
	auto iter = str;
	while (*iter != '\0')
	{
		iter++;
	}
	auto size = iter - str;
	m_Size = size + 1;
	m_Data = std::make_unique<char[]>(m_Size);
	std::copy(str, str + m_Size, m_Data.get());
	m_Data[m_Size - 1] = '\0';
}

String::String(const String& string) 
{
	m_Size = string.m_Size;
	m_Data = std::make_unique<char[]>(string.m_Size);
	std::copy(string.m_Data.get() ,string.m_Data.get() + m_Size,
		 m_Data.get());
}
String::String(String&& string)
{
	m_Data = std::move(string.m_Data);
	m_Size = string.m_Size;
	string.m_Size = 0;
}

String::~String()
{
	m_Data.reset();
}

bool String::operator==(const String& str) const
{
	return AreEqual(m_Data.get(), GetSize(), str.GetRaw(), str.GetSize());
}
bool String::operator==(const char* str)
{
	return strcmp(m_Data.get(), str) == 0;
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
		m_Size = string.m_Size;
		m_Data = std::make_unique<char[]>(string.m_Size);
		std::copy(string.m_Data.get(), string.m_Data.get() + m_Size,
			m_Data.get());
	}
	
	return *this;
}

