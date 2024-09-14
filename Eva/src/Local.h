#pragma once
#include"String.hpp"
#include"Nodes.h"
#include<array>
#include<vector>
struct Local
{
	int depth;
	String name;
};
struct StackSim
{
	// so we can access previous scopes too
	std::vector<const Scope*> currentScopes;
	std::array<Local, 256> locals;
	// track the declared locals
	int m_StackPtr = 0;

	std::string_view LastLocal()
	{
		return locals[m_StackPtr - 1].name.GetStringView();
	}

	std::tuple<bool, int> IsLocalExist(String& name, size_t scope)
	{
		auto temp = m_StackPtr - 1;
		while (temp >= 0)
		{
			if (name == locals[temp].name && scope >= locals[temp].depth)
			{
				return { true ,temp };
			}
			temp--;
		}
		return { false ,-1 };
	}


};
