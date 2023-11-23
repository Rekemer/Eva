#pragma once
#include "object.h"
#include <ostream>
class String  : public Object
{
public:
	String() :m_Size{ 0 }, m_Str{nullptr} {

	};
	String(const char* str)
	{
		auto iter = str;
		while (*iter != '\0')
		{
			iter++;
		}
		auto size = iter -str;
		m_Size = size+1;
		m_Str = new char[m_Size];
		memcpy(m_Str, str, m_Size * sizeof(char));
		m_Str[m_Size - 1] = '\0';
	}
	const char* GetRaw()
	{
		return m_Str;
	}
	int GetSize()
	{
		return m_Size - 1;
	}
	String(const char* str, size_t size);
	String(const String& string);
	String(String&& string);
	String& operator = (const String& string);
	~String();
	bool operator == (const String& str);
private:
	friend std::ostream& operator<<(std::ostream& os, String& string);
	// inlcudes zero at end
	int m_Size;
	char* m_Str;
};
inline std::ostream& operator<<(std::ostream& os, String& string)
{
	os << string.m_Str;
	return os;
}