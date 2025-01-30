#pragma once
#include <windows.h>
// they pollute my namespace!
// conflicts with Eva::InCode::TRUE and etc
#undef TRUE
#undef FALSE
#undef ERROR
#undef THIS



#include <iostream>
#include <string>
#include <string_view>
inline HMODULE LoadDll(std::string_view path)
{
    auto wStr = std::wstring(path.begin(), path.end());
    // Load the DLL
    HMODULE hDLL = LoadLibrary(wStr.data());  // Replace with your DLL name
    if (!hDLL) {
        std::cerr << "Failed to load DLL!" << std::endl;
        return nullptr;
    }
    return hDLL;
}
inline void FreeLib(HMODULE handle)
{
    FreeLibrary(handle);
}

template<typename T>
T LoadFunc(HMODULE libHandle, std::string_view funcName)
{
    auto func = (T)GetProcAddress(libHandle, "funcName");
    if (!func) {
        std::cerr << "Failed to get function address!" << std::endl;
        return nullptr;
    }
    return func;

}