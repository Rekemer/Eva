#pragma once
#include "String.hpp"
#include "Value.h"
#include <vector>
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


class HashTable
{
public:
	HashTable()
	{
		m_Data = new Entry[m_Size];
	}
	~HashTable()
	{
		delete[]m_Data;
	}
	void Add(const Entry& entry);
	bool IsExist(const char* key);
	void Add(const char* key, ValueContainer value);
	void Delete(const Entry& entry);
	Entry* Get(String* key);
private:
	Entry* FindEntry(Entry* data,String* key, int amountOfData);

	void MakeTombstone(Entry* entry);

	// entries + tombstones
	int m_EntriesAmount = 0;
	int m_Size = 8;
	Entry* m_Data = nullptr;
	// should benchmark and tune this 
	const float TABLE_MAX_LOAD = 0.75;
};