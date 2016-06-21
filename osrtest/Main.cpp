// osrtest.cpp : Defines the entry point for the console application.
//

#include "Precomp.h"

#include <strsafe.h>

int main()
{
    wprintf_s(L"Finding device interfaces...\n");

    auto list = wtl::cm::get_device_interface_list(GUID_DEVINTERFACE_OSR_FX2);

    if (list)
    {
        for (auto devInterface : list.get())
        {
            wprintf_s(L"DI: %ls\n", devInterface);

            auto filePath = std::wstring(devInterface) + L"\\DIP_SWITCHES";

            auto mHandle = wtl::file::create(filePath.c_str(), GENERIC_READ | GENERIC_WRITE);

            if (!mHandle)
            {
                wprintf_s(L"Failed to open %ls with error %u\n", filePath.c_str(), mHandle.get_result());
            }
        }
    }
    else
    {
        wprintf_s(L"Error: Failed with 0x%08X\n", list.get_result());
    }

    wprintf_s(L"Exiting...\n");

    return 0;
}

