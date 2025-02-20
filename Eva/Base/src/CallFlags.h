#pragma once

namespace Eva
{
	enum class CallFlags : unsigned {
		UserFunc = 1,
		BuiltIn = 1 << 1,
		ExternalDLL = 1 << 2,
		VoidCall = 1 << 3
	};




	CallFlags operator|(CallFlags lhs, CallFlags rhs);

	CallFlags& operator|=(CallFlags& lhs, CallFlags rhs);

	CallFlags operator&(CallFlags lhs, CallFlags rhs);

	CallFlags& operator&=(CallFlags& lhs, CallFlags rhs);


	bool HasFlag(CallFlags flags, CallFlags test);
}
