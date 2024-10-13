#include"HashTable.h"
#include<iostream>

void HashTable::Delete(std::string_view key)
{
	auto retrievedEntry = FindEntry(m_Data.get(), key, m_Size);
	MakeTombstone(retrievedEntry);
}

void HashTable::MakeTombstone(Entry* entry)
{
	entry->key = "";
	entry->value = true;
}

bool HashTable::IsExist(std::string_view key) const
{
	auto entry = Get(key);
	return IsSet(entry);
}
void HashTable::Print()
{
	auto index = 0;
	while (index < m_Size)
	{
		auto data = &m_Data[index];
		if (IsSet(data))
		{
			std::cout << data->key << std::endl;
		}
		index +=1;
	}
}
Entry* HashTable::Get(std::string_view key) const
{
	return FindEntry(m_Data.get(), key, m_Size);
}


// works for insertion and looking up
Entry* HashTable::FindEntry(Entry* data, std::string_view key, int amountOfData)  const
{
	auto hash = HashString(key.data(), key.size());
	auto index = hash % (amountOfData );
	
	Entry* tombstone = nullptr;

	while (true)
	{
		auto* entry = (data + index);
		if (IsSet(entry))
		{
			bool isSameKey = entry->key == key;
			if (isSameKey)
			{
				return entry;
			}
		}


		if (!entry->IsInit() && tombstone == nullptr)
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
