#pragma once
#include "object.h"
#include <ostream>

class String  : public Object
{
public:
	String() :m_Size{ 0 }, m_Data{nullptr} {

	};
	
	String(const char* str);
	String(const char* str, size_t size);
	template<typename ... Args>
	String(const char* arg1, Args... args);
	String(const String& string);
	String(String&& string);
	~String();
	static bool AreEqual(const char* str,size_t size, const char* str2, size_t size2);
	const char* GetRaw() const
	{
		return m_Data.get();
	}
	std::string_view GetStringView() const
	{
		return { m_Data.get(),static_cast<size_t>(m_Size - 1)};
	}
	int GetSize() const
	{
		return m_Size - 1;
	}
	String& operator = (const String& string);
	bool operator == (const String& str) const ;
	bool operator == (const char* str);
	bool operator != (const char* str);
private:
	friend std::ostream& operator<<(std::ostream& os, String& string);
	// inlcudes zero at end
	int m_Size;
	std::unique_ptr<char[]> m_Data = nullptr;
};
inline std::ostream& operator<<(std::ostream& os, String& string)
{
	os << string.m_Data;
	return os;
}

namespace std
{
	template<>
	struct hash<String>
	{
		size_t operator()(const String& s) const
		{
			//return std::hash<std::string>{}(s.GetRaw());
			return std::hash<const char*>()(s.GetRaw());
		}
	};

}