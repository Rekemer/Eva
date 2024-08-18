#include"HashTable.h"

void HashTable::Delete(std::string_view key)
{
	auto retrievedEntry = FindEntry(m_Data.get(), key, m_Size);
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
