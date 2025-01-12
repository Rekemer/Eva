#pragma once
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include "Function.h"
namespace cereal
{
#define BIN 1 
	template<class Archive>
	void serialize(Archive& archive,
		Func& func)
	{

		auto& constPool = func.constants;
		auto& bytecode = func.opCode;
#if BIN
		archive(constPool);
		archive(bytecode);
#else
		archive(CEREAL_NVP(constPool), CEREAL_NVP(bytecode));
#endif
	}

	template<class Archive>
	void serialize(Archive& archive,
		ValueContainer& v)
	{
		

#if BIN
		archive(v.type);
		archive(v.as);
#else
		archive(CEREAL_NVP(v.type));
		archive(CEREAL_NVP(v.as));
#endif
	}
}