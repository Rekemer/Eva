#pragma once
#include "win_header.h"

#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <optional>
#include <string_view>
#include "Value.h"
#include "PluginData.h"
#include "CallState.h"

namespace Eva
{
    using DLLHandle = HMODULE;
   


    std::pair<bool, std::string_view> IsPlugin(std::string_view functionName, std::unordered_map<std::string, PluginData>& plugins);
    DLLHandle CastToModule(void* ptr);
    inline ValueType GetReturnTypeFromPlugin(std::unordered_map<std::string, PluginData>& plugins, std::string_view pluginName, std::string_view name)
    {
        auto& data = plugins.at(pluginName.data());
        auto type = data.typeMap->at(name.data());
        return type;
    }
    // checks all plugins
    inline ValueType GetReturnTypeDLL(std::unordered_map<std::string, PluginData>& plugins, std::string_view name)
    {
        for (auto& [k, v] : plugins)
        {
            if (v.typeMap->find(name.data()) != v.typeMap->end())
            {
                return  v.typeMap->at(name.data());
            }
        }
        assert(false);
    }
  

    template<typename T>
    T LoadFunc(DLLHandle libHandle, std::string_view funcName)
    {
        auto func = (T)GetProcAddress(libHandle, funcName.data());
        if (!func) {
            std::cerr << "Failed to get function address!" << std::endl;
            return nullptr;
        }
        return func;

    }

    inline std::shared_ptr<ICallable> GetCall(std::string_view name, HMODULE mod)
    {
        using GetCallSign = std::shared_ptr<ICallable>(*)(const char* name);
        auto getCall = LoadFunc<GetCallSign>(mod, "GetCallable");
        return getCall(name.data());
    }



  
    inline DLLHandle LoadDllPtr(std::string_view path)
    {
        auto wStr = std::wstring(path.begin(), path.end());
        // Load the DLL
        DLLHandle hDLL = LoadLibrary(wStr.data());
        return hDLL;
    }
    using InitSignature = std::unordered_map<std::string, Eva::ValueType>*(*)();

    inline std::optional<PluginData> LoadDll(std::string_view path)
    {
        // Load the DLL
        auto  hDLL = LoadDllPtr(path);
        auto init = LoadFunc<InitSignature>(hDLL, "initModule");
        auto map = init();
        auto data  = PluginData{ path.data(),hDLL,std::unique_ptr<PluginReturn>{ map }};
        return data;
    }


    inline void FreeLib(DLLHandle handle)
    {
        FreeLibrary(handle);
    }

}