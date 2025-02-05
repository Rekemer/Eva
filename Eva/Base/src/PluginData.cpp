#include "PluginData.h"
#include "win_header.h"
#include "PluginLoader.h"
namespace Eva
{
	PluginData::~PluginData()
	{
		FreeLibrary(CastToModule(hDLL));
	}



}