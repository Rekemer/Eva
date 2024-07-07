#pragma once
#include "String.hpp"
#include "Value.h"
#include <vector>
#include <utility>
// FHV-1a
static uint32_t HashString(const char* key, int length) {
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; i++) {
		hash ^= key[i];
		hash *= 16777619;
	}
	return hash;
}

struct Entry
{
	String* key = nullptr;
	ValueContainer value;
};

inline bool IsTombstone(Entry* entry)
{
	return entry->key == nullptr && entry->value.As<bool>() == true;
}
inline bool IsSet(Entry* entry)
{
	return entry != nullptr && entry->key != nullptr;
}
inline bool IsNotSet(Entry* entry)
{
	return entry->key == nullptr && entry->value.type == ValueType::NIL;
}

class HashTable
{
public:
	HashTable()
	{
		m_Data = new Entry[m_Size];
	}
	HashTable(const HashTable& table)
	{
		m_Data = new Entry[table.m_Size];
		m_Size = table.m_Size;
		m_EntriesAmount = table.m_EntriesAmount;
		memcpy(m_Data,table.m_Data, table.m_Size * sizeof(Entry));


	}
	HashTable& operator = (const HashTable& table)
	{
		if (&table == this) return *this;
		m_Data = new Entry[table.m_Size];
		m_Size = table.m_Size;
		m_EntriesAmount = table.m_EntriesAmount;
		memcpy(m_Data, table.m_Data, table.m_Size * sizeof(Entry));
	}
	~HashTable()
	{
		delete[]m_Data;
	}
	void Add(const Entry& entry);
	bool IsExist(std::string_view key);
	template<class... Arg>
	Entry* Add(std::string_view key, Arg... value)
	{
		auto loadFactor = static_cast<float>(m_EntriesAmount) / m_Size;
		// resize array
		if (loadFactor > TABLE_MAX_LOAD)
		{
			auto oldData = m_Data;
			m_Size *= 2;
			m_Data = new Entry[m_Size];

			auto iter = oldData;


			// we need recalculate since we are rebuilding hash map 
			// and tombstones are no longer relevant
			m_EntriesAmount = 0;

			for (int i = 0; i < m_Size/2; i++)
			{
				if (oldData[i].key != nullptr)
				{
					auto retrievedEntry = FindEntry(m_Data, oldData[i].key->GetRaw(), m_Size/2);
					retrievedEntry->key = oldData[i].key;
					retrievedEntry->value = oldData[i].value;
					m_EntriesAmount++;
				}
			}
			delete[] oldData;
		}
		auto retrievedEntry = FindEntry(m_Data, key, m_Size);

		if (IsNotSet(retrievedEntry))
		{
			m_EntriesAmount++;
		}

		retrievedEntry->key = new String(key.data(), key.size());
		retrievedEntry->value = ValueContainer{ value... };
		return retrievedEntry;
	}
	void Delete(std::string_view key);
	Entry* Get(std::string_view key)const;
private:
	Entry* FindEntry(Entry* data,std::string_view key, int amountOfData) const;

	void MakeTombstone(Entry* entry);

	// entries + tombstones
	int m_EntriesAmount = 0;
	int m_Size = 8;
	Entry* m_Data = nullptr;
	// should benchmark and tune this 
	const float TABLE_MAX_LOAD = 0.75;
};