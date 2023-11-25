#pragma once
#include "object.h"
#include <ostream>
class String  : public Object
{
public:
	String() :m_Size{ 0 }, m_Data{nullptr} {

	};
	
	//String(const char* str);
	String(const char* str, size_t size);
	template<typename ... Args>
	String(const char* arg1, Args... args);
	String(const String& string);
	String(String&& string);
	~String();
	static bool AreEqual(const char* str,size_t size, const char* str2, size_t size2);
	const char* GetRaw() const
	{
		return m_Data;
	}
	int GetSize() const
	{
		return m_Size - 1;
	}
	String& operator = (const String& string);
	bool operator == (const String& str);
private:
	friend std::ostream& operator<<(std::ostream& os, String& string);
	// inlcudes zero at end
	int m_Size;
	char* m_Data;
};
inline std::ostream& operator<<(std::ostream& os, String& string)
{
	os << string.m_Data;
	return os;
}

template<typename ...Args>
inline String::String(const char* arg1, Args ...args)
{
	size_t totalLength = strlen(arg1) + (strlen(args) + ...);
	// Allocate memory for the concatenated string
	m_Size = totalLength + 1;
	m_Data = new char[m_Size];
	// Copy the first argument into the buffer
	strcpy(m_Data, arg1);

	// Concatenate the rest of the arguments
	(strcat(m_Data, args), ...);
}
