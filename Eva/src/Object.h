#pragma once
// heap allocated objects

enum class ObjectType
{
	STRING
};

class Object
{
public:
	ObjectType type;
};

class String : public Object
{
public:
	String(const char* str, int size);
	~String();
private:
	int m_size;
	char* m_str;
};