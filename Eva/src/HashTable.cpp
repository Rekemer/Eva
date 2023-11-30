#include"HashTable.h"

bool IsTombstone(Entry* entry)
{
	return entry->key == nullptr && entry->value.As<bool>() == true;
}
bool IsSet(Entry* entry)
{
	return entry->key != nullptr;
}
bool IsNotSet(Entry* entry)
{
	return entry->key == nullptr && entry->value.type== ValueType::NIL;
}


Entry* HashTable::Add(std::string_view key,ValueContainer value)
{
	auto loadFactor = m_EntriesAmount / m_Size;
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

		for (int i = 0; i < m_Size; i++)
		{
			if (oldData[i].key)
			{
				auto retrievedEntry = FindEntry(m_Data,oldData[i].key->GetRaw(), m_Size);
				retrievedEntry->key = oldData[i].key;
				retrievedEntry->value = oldData[i].value;
				m_EntriesAmount++;
			}
		}
		delete[] oldData;
	}
	auto retrievedEntry = FindEntry(m_Data,key,m_Size);
	
	if (IsNotSet(retrievedEntry))
	{
		m_EntriesAmount++;
	}

	retrievedEntry->key = new String(key.data(),key.size());
	retrievedEntry->value= value;
	return retrievedEntry;
}

void HashTable::Delete(std::string_view key)
{
	auto retrievedEntry = FindEntry(m_Data,key,m_Size);
	MakeTombstone(retrievedEntry);
}



void HashTable::MakeTombstone(Entry* entry)
{
	entry->key = nullptr;
	entry->value = true;
}


bool HashTable::IsExist(std::string_view key)
{
	auto entry = Get(key);
	return IsSet(entry);
}

Entry* HashTable::Get(std::string_view key)
{
	return FindEntry(m_Data,key, m_Size);
}


// works for insertion and looking up
Entry* HashTable::FindEntry(Entry* data, std::string_view key, int amountOfData)
{
	auto hash = HashString(key.data(), key.size());
	auto index = hash % amountOfData;
	
	Entry* tombstone = nullptr;

	while (true)
	{
		auto* entry = (data + index);
		
		if (IsSet(entry))
		{
			bool isSameKey = String::AreEqual(entry->key->GetRaw(),
				entry->key->GetSize(), key.data(), key.size());
			if (isSameKey)
			{
				return entry;
			}
		}


		if (entry->key == nullptr && tombstone == nullptr)
		{
			return &data[index];
		}
		else if (IsNotSet(entry) && tombstone)
		{
			return tombstone;
		}
		else if (IsTombstone(entry))
		{
			/*
				we set tombstone to return it in the future
				in case if there are no further entries with key we need
			*/
			tombstone = entry;
		}
		else
		{
			index += 1;
			index %= amountOfData;
		}
	}
}
