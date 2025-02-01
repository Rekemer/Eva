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

namespace Eva
{

    template<typename T>
    T LoadFunc(HMODULE libHandle, std::string_view funcName)
    {
        auto func = (T)GetProcAddress(libHandle, funcName.data());
        if (!func) {
            std::cerr << "Failed to get function address!" << std::endl;
            return nullptr;
        }
        return func;

    }
    using InitSignature = std::unordered_map<std::string, Eva::ValueType>*(*)();

    inline std::optional<PluginData> LoadDll(std::string_view path)
    {
        auto wStr = std::wstring(path.begin(), path.end());
        auto check = std::filesystem::exists(wStr);
        // Load the DLL
        HMODULE hDLL = LoadLibrary(wStr.data());
        if (!hDLL) {
            std::cerr << "Failed to load DLL!" << std::endl;
            return {};
        }

        auto init = LoadFunc<InitSignature>(hDLL, "initModule");
        auto map = init();
        auto data  = PluginData{ path.data(),hDLL,std::unique_ptr<PluginReturn>{ map }};
        return data;
    }
    inline void FreeLib(HMODULE handle)
    {
        FreeLibrary(handle);
    }

}