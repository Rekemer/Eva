#pragma once
#include "Value.h"
#include <string>
#include <memory>
#include <unordered_map>
namespace Eva
{
    // first element is return type, next are args
    using TypeTable = std::unordered_map<std::string, std::vector<Eva::ValueType>>;
    // might be not integers potentially
    using ConstTable = std::unordered_map<std::string, eint>;
   
    struct PluginData
    {
        std::string name;
        void* hDLL;
        std::unique_ptr<TypeTable> typeMap;
        ConstTable* constMap = nullptr;
        // for cereal
        PluginData() = default;

        PluginData(const std::string& name, void* hDLL, std::unique_ptr<TypeTable> typeMap)
        {
            this->name = name;
            this->hDLL = hDLL;
            this->typeMap = std::move(typeMap);
        }

        PluginData(PluginData&& other) : typeMap(std::move(other.typeMap))
        {
            name = std::move(other.name);
            hDLL = other.hDLL;
            constMap = other.constMap;
            other.hDLL  = other.constMap = nullptr;
        }

        PluginData& operator=(PluginData&& other) {
            
            if (this != &other) {
                name = std::move(other.name);
                hDLL = other.hDLL;
                other.hDLL = nullptr;
                typeMap = std::move(other.typeMap);
                constMap = other.constMap;
                other.constMap = nullptr;
            }
            return *this;
        }

        PluginData(PluginData&) = delete;
        PluginData& operator=(const PluginData& other) = delete;

        ~PluginData();
    };
    using PluginTable = std::unordered_map <std::string, PluginData > ;
}
