#pragma once
#include <vector>
#include <string>
#include "Value.h"
#include "CallFlags.h"
namespace Eva
{
	struct CallState;
	class ICallable
	{
	public:
		const inline static int INF_ARGS = -2;

		ICallable() = default;

		ICallable(std::vector<ValueType> argTypes, int argCount, std::string_view name, CallFlags callFlags)
			:argTypes{ argTypes },
			argCount{ argCount },
			name{ name.data() },
			callFlags{ callFlags }
		{

		}

		bool IsArgUnlimited()
		{
			return argCount == INF_ARGS;
		}
		virtual size_t Call(CallState& callState,  size_t baseIndex) = 0;
		int argCount = 0;
		std::vector<ValueType> argTypes;
		std::string name;
		CallFlags callFlags = CallFlags::BuiltIn;
		virtual ~ICallable()
		{

		};
	};
}




