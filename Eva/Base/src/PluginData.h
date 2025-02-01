#pragma once
#include "Value.h"
#include <string>
#include <memory>
#include <unordered_map>
namespace Eva
{
    using PluginReturn = std::unordered_map<std::string, Eva::ValueType>;
    struct PluginData
    {
        std::string name;
        void* hDLL;
        std::unique_ptr<PluginReturn> typeMap;

        PluginData(const std::string& name, void* hDLL, std::unique_ptr<PluginReturn> typeMap)
        {
            this->name = name;
            this->hDLL = hDLL;
            this->typeMap = std::move(typeMap);
        }

        PluginData(PluginData&& other) : typeMap(std::move(other.typeMap)) 
        {
            name = std::move(other.name);
            hDLL = other.hDLL;
            other.hDLL = nullptr;
        }

        PluginData& operator=(PluginData&& other) {
            
            if (this != &other) {
                name = std::move(other.name);
                hDLL = other.hDLL;
                other.hDLL = nullptr;
                typeMap = std::move(other.typeMap);
            }
            return *this;
        }

        PluginData(PluginData&) = delete;

        ~PluginData();
    };
}
