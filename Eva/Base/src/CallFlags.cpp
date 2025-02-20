#include "CallFlags.h"
#include <type_traits>
namespace Eva
{

	 CallFlags operator|(CallFlags lhs, CallFlags rhs) {
		using T = std::underlying_type_t<CallFlags>;
		return static_cast<CallFlags>(
			static_cast<T>(lhs) | static_cast<T>(rhs)
			);
	}

	 CallFlags& operator|=(CallFlags& lhs, CallFlags rhs) {
		lhs = lhs | rhs;
		return lhs;
	}

	 CallFlags operator&(CallFlags lhs, CallFlags rhs) {
		using T = std::underlying_type_t<CallFlags>;
		return static_cast<CallFlags>(
			static_cast<T>(lhs) & static_cast<T>(rhs)
			);
	}

	 CallFlags& operator&=(CallFlags& lhs, CallFlags rhs) {
		lhs = lhs & rhs;
		return lhs;
	}


	 std::underlying_type_t<CallFlags> CallFlagsToInt(CallFlags flags) {
		return static_cast<std::underlying_type_t<CallFlags>>(flags);
	}

	 CallFlags IntToCallFlags(std::underlying_type_t<CallFlags> value) {
		return static_cast<CallFlags>(value);
	}

	 bool operator==(CallFlags lhs, CallFlags rhs) {
		using T = std::underlying_type_t<CallFlags>;
		return static_cast<T>(lhs) == static_cast<T>(rhs);
	}

	 bool operator!=(CallFlags lhs, CallFlags rhs) {
		return !(lhs == rhs);
	}


	 bool HasFlag(CallFlags flags, CallFlags test) {
		 return (CallFlagsToInt(flags & test) != 0);
	 }
}
