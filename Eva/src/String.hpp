#pragma once
#include "object.h"
#include <ostream>
class String  : public Object
{
public:
	String() :m_Size{ 0 }, m_Str{nullptr} {

	};
	
	//String(const char* str);
	String(const char* str, size_t size);
	String(const String& string);
	String(String&& string);
	~String();
	static bool AreEqual(const char* str,size_t size, const char* str2, size_t size2);
	const char* GetRaw() const
	{
		return m_Str;
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
	char* m_Str;
};
inline std::ostream& operator<<(std::ostream& os, String& string)
{
	os << string.m_Str;
	return os;
}