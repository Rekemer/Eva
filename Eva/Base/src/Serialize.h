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
#include <cereal/types/polymorphic.hpp>
#include "Function.h"
#include "ICallable.h"
#include "Entry.h"
#include "HashTable.h"
#include "PluginData.h"


CEREAL_REGISTER_TYPE(Eva::Func)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Eva::ICallable, Eva::Func)

inline bool HasJsonExtension(std::string_view str)	
{
// Ensure the string is at least 5 characters (".json")
	if (str.size() < 5) {
		return false;
	}
	// Check if the last 5 characters match ".json"
	return str.substr(str.size() - 5) == ".json";
}

namespace cereal
{
	template <typename Archive>
	constexpr bool is_binary_archive_v =
		std::is_base_of_v<cereal::BinaryInputArchive, Archive> ||
		std::is_base_of_v<cereal::BinaryOutputArchive, Archive>;


	template <class Archive>
	void serialize(Archive& archive, Eva::ICallable& func)
	{
		archive(
			cereal::make_nvp("argCount", func.argCount),
			cereal::make_nvp("argTypes", func.argTypes),
			cereal::make_nvp("name", func.name)
		);
	}

	template <class Archive>
	void serialize(Archive& archive, Eva::Func& func) {
		auto& constPool = func.constants;
		auto& bytecode = func.opCode;
		archive(cereal::virtual_base_class<Eva::ICallable>(&func));
		if constexpr (is_binary_archive_v<Archive>) {
			// Binary: no name-value pairs
			archive(constPool, bytecode);
		}
		else {
			// JSON, XML, or other text-based archive
			archive(
				cereal::make_nvp("name", func.name),
				cereal::make_nvp("constants", constPool),
				cereal::make_nvp("opCode", bytecode)
			);
		}
	}
	template<class Archive>
	void serialize(Archive& archive,
		Eva::ValueContainer& v)
	{
		if constexpr (is_binary_archive_v<Archive>)
		{
			archive(v.type);
			archive(v.as);
		}
		else
		{
			archive(CEREAL_NVP(v.type));
			archive(CEREAL_NVP(v.as));
		}
	}
	
	
	inline void saveEntry ( Eva::Entry* entry, int i, std::vector<Eva::Entry>& existingEntries)
		{
			if (IsSet(entry + i))
			{
				existingEntries.push_back(entry[i]);
				//if constexpr (is_binary_archive_v<Archive>)
				//archive(cereal::make_nvp("Entry_" + std::to_string(i), entry[i]));
				//else
				//archive(entry[i]);
			}
		};
	template<typename Archive>
	void loadEntry(Archive& archive, Eva::HashTable& v)
	{
		std::vector<Eva::Entry> existingEntries;
		if constexpr (is_binary_archive_v<Archive>)
		{
			archive(existingEntries);
			for (const auto& const e : existingEntries)
			{
				v.Add(e);
			}
		}
		else
		{
			std::vector<Eva::Entry> existingEntries;
			archive(cereal::make_nvp("entries", existingEntries));
			for (auto& e : existingEntries)
			{
				v.Add(e);
			}
		}
	};
	template <class Archive>
	void save(Archive& archive, const Eva::HashTable& v)
	{
		std::vector<Eva::Entry> existingEntries;
		// Common fields
		if constexpr (is_binary_archive_v<Archive>) {
			// Binary output: just write raw data, no naming
			archive(v.m_EntriesAmount, v.m_Size);

			for (int i = 0; i < v.m_Size; ++i) {

				saveEntry(v.m_Data.get(), i, existingEntries);
			}
			archive(existingEntries);
		}
		else {
			// JSON or other text-based archive: use name-value pairs
			archive(
				CEREAL_NVP(v.m_EntriesAmount),
				CEREAL_NVP(v.m_Size)
			);

			for (int i = 0; i < v.m_Size; ++i) {
				saveEntry(v.m_Data.get(), i, existingEntries);
			}
			archive(cereal::make_nvp("entries", existingEntries));
		}
	}

	template <class Archive>
	void load(Archive& archive, Eva::HashTable& v)
	{
		if constexpr (is_binary_archive_v<Archive>) {
			// Binary input
			archive(v.m_EntriesAmount, v.m_Size);

			// Allocate array for entries
			v.m_Data = std::make_unique<Eva::Entry[]>(v.m_Size);
			loadEntry(archive, v);
			// Load entries
			//for (int i = 0; i < v.m_Size; ++i) {
			//	archive(v.m_Data[i]);
			//}
		}
		else {
			// JSON (or other text-based)
			archive(
				CEREAL_NVP(v.m_EntriesAmount),
				CEREAL_NVP(v.m_Size)
			);
			v.m_Data = std::make_unique<Eva::Entry[]>(v.m_Size);
			loadEntry(archive, v);

			//for (int i = 0; i < v.m_Size; ++i) {
			//	archive(cereal::make_nvp("Entry_" + std::to_string(i), v.m_Data[i]));
			//}
		}
	}

	// entry of  hashTable
	template<class Archive>
	void serialize(Archive& archive,
		Eva::Entry& v)
	{
		if constexpr (is_binary_archive_v<Archive>)
		{
			archive(v.key);
			archive(v.value);
		}
		else
		{
			archive(CEREAL_NVP(v.key));
			archive(CEREAL_NVP(v.value));
		}
	}

	template<class Archive>
	void serialize(Archive& archive,
		Eva::PluginData& v)
	{
		if constexpr (is_binary_archive_v<Archive>)
		{
			archive(v.name);
			//archive(v.hDLL);
			archive(v.typeMap);
		}
		else
		{
			archive(CEREAL_NVP(v.name));
			//archive(CEREAL_NVP(v.hDLL));
			archive(CEREAL_NVP(v.typeMap));
		}
	}

}