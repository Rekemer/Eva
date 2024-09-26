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
	std::shared_ptr<String> key = nullptr;
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
	template<class T>
	class HashTableIterator
	{
		using PtrType = T*;
		using RefType = T&;
	public:
		HashTableIterator(PtrType  data, PtrType end) : m_Data{ data }, m_End{end}
		{

		}
		static PtrType GetValid(PtrType ptr, PtrType end)
		{
			while (!IsSet(ptr) && ptr != end)
			{
				++ptr;
			}
			return ptr;
		}
		// prefix operator
		HashTableIterator& operator++()
		{
			++m_Data;
			m_Data = GetValid(m_Data, m_End);
			return *this;
		}
		// prefix operator
		HashTableIterator& operator--()
		{
			--m_Data;
			return this;
		}
		// postfix operator
		HashTableIterator operator++(int)
		{
			HashTableIterator next = *this;
			++(*this);
			return *next;
		}
		// postfix operator
		HashTableIterator operator--(int)
		{
			HashTableIterator next = *this;
			--(*this);
			return *next;
		}
		PtrType operator ->()
		{
			return m_Data;
		}
		RefType operator * ()
		{
			return *m_Data;
		}
		RefType operator [](int index)
		{
			return *(m_Data + index);
		}
		bool operator == (HashTableIterator other)
		{
			return other.m_Data == m_Data;
		}
		bool operator != (HashTableIterator other)
		{
			return !(other.m_Data == m_Data);
		}
	private:
		PtrType  m_Data;
		// without it we have ub since we read wrong memory
		PtrType  m_End;

	};
public:
	using Iterator = HashTableIterator<Entry>;
	Iterator begin()
	{
		auto ptr = m_Data.get();
		ptr = Iterator::GetValid(ptr, m_Data.get() + m_Size);
		return Iterator{ptr,m_Data.get() + m_Size };
		//return Iterator{m_Data.get()};
	}
	Iterator end()
	{
		return Iterator{m_Data.get() + m_Size,m_Data.get() + m_Size };
	}

	HashTable()
	{
		m_Data = std::make_unique<Entry[]>(m_Size);
	}
	HashTable(const HashTable& table)
	{
		m_Data = std::make_unique<Entry[]>(table.m_Size);
		m_Size = table.m_Size;
		m_EntriesAmount = table.m_EntriesAmount;
		std::copy(table.m_Data.get(), table.m_Data.get() + table.m_Size, m_Data.get()); 
	}
	void Print();
	HashTable& operator = (const HashTable& table)
	{
		if (&table == this) return *this;
		m_Data.reset();
		m_Data = std::make_unique<Entry[]>(table.m_Size);
		m_Size = table.m_Size;
		m_EntriesAmount = table.m_EntriesAmount;
		std::copy(table.m_Data.get(), table.m_Data.get() + table.m_Size, m_Data.get()); 
	}
	void Add(const Entry& entry);
	bool IsExist(std::string_view key) const;
	template<class... Arg>
	Entry* Add(std::string_view key, Arg... value)
	{
		auto loadFactor = static_cast<float>(m_EntriesAmount) / m_Size;
		// resize array
		if (loadFactor > TABLE_MAX_LOAD)
		{
			auto oldData = std::move(m_Data);
			m_Size *= 2;
			m_Data = std::make_unique<Entry[]>(m_Size);
			
			// we need recalculate since we are rebuilding hash map 
			// and tombstones are no longer relevant
			m_EntriesAmount = 0;

			for (int i = 0; i < m_Size/2; i++)
			{
				if (oldData[i].key != nullptr)
				{
					auto retrievedEntry = FindEntry(m_Data.get(), oldData[i].key->GetRaw(), m_Size);
					retrievedEntry->key = oldData[i].key;
					retrievedEntry->value = oldData[i].value;
					m_EntriesAmount++;
				}
			}
		}
		auto retrievedEntry = FindEntry(m_Data.get(), key, m_Size);

		if (IsNotSet(retrievedEntry))
		{
			m_EntriesAmount++;
		}

		retrievedEntry->key = std::make_shared<String>(key.data(), key.size());
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
	std::unique_ptr<Entry[]> m_Data = nullptr;
	// should benchmark and tune this 
	const float TABLE_MAX_LOAD = 0.75;
};