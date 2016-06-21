// osrtest.cpp : Defines the entry point for the console application.
//

#include "Precomp.h"

#include <strsafe.h>

using namespace wtl;

hresult hr_main()
{
    wprintf_s(L"Finding device interfaces...\n");

    auto list = hresult_from_configret(
        cm::get_device_interface_list(GUID_DEVINTERFACE_OSR_FX2));
    if (!list) return list;

    for (auto devInterface : list.get())
    {
        wprintf_s(L"DI: %ls\n", devInterface);

        auto filePath = std::wstring(devInterface) + L"\\DIP_SWITCHES";

        auto file = file::create(filePath.c_str(), GENERIC_READ | GENERIC_WRITE);

        if (!file)
        {
            wprintf_s(L"Failed to create %ls with error %u\n", filePath.c_str(), file.get_result());
        }
    }

    return S_OK;
}

int main()
{
    auto mainResult = hr_main();

    if (!mainResult)
    {
        wprintf_s(L"Error: Failed with 0x%08X\n", mainResult.get_result());
    }

    wprintf_s(L"Exiting...\n");

    return 0;
}

