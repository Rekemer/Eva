#pragma once
enum class CallFlags : unsigned {
	UserFunc = 0,
	BuiltIn = 1 << 0,
	ExternalDLL = 1 << 1
};

inline CallFlags operator|(CallFlags lhs, CallFlags rhs) {
	using T = std::underlying_type_t<CallFlags>;
	return static_cast<CallFlags>(
		static_cast<T>(lhs) | static_cast<T>(rhs)
		);
}

inline CallFlags& operator|=(CallFlags& lhs, CallFlags rhs) {
	lhs = lhs | rhs;
	return lhs;
}

inline CallFlags operator&(CallFlags lhs, CallFlags rhs) {
	using T = std::underlying_type_t<CallFlags>;
	return static_cast<CallFlags>(
		static_cast<T>(lhs) & static_cast<T>(rhs)
		);
}

inline CallFlags& operator&=(CallFlags& lhs, CallFlags rhs) {
	lhs = lhs & rhs;
	return lhs;
}

inline bool operator==(CallFlags lhs, CallFlags rhs) {
	using T = std::underlying_type_t<CallFlags>;
	return static_cast<T>(lhs) == static_cast<T>(rhs);
}

inline bool operator!=(CallFlags lhs, CallFlags rhs) {
	return !(lhs == rhs);
}
