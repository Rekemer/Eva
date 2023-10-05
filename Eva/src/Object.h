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
	virtual ~Object() = 0;
};



