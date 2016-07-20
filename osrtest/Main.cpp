// osrtest.cpp : Defines the entry point for the console application.
//

#include "Precomp.h"

#include <strsafe.h>
#include <signal.h>
#include <iostream>
#include <array>

using namespace wtl;

multi_sz get_device_interfaces()
{
    return as_hr(cm::get_device_interface_list(GUID_DEVINTERFACE_OSR_FX2)).value();
}

file get_dip_switches_file()
{
    for (auto devIf : get_device_interfaces())
    {
        auto filePath = std::wstring(devIf) + L"\\" DIP_SWITCHES_FILE;

        auto file = file::create(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED);
        if (!file) continue;

        return std::move(file.get());
    }

    throw result_exception<HRESULT>(E_NOINTERFACE);
}

void listen()
{
    static auto signalInterrupt = event::create().value();

    signal(SIGINT, [](int signal) {
        signalInterrupt.set();
    });

    auto file = get_dip_switches_file();

    auto readEvent = event::create().value();

    while (true)
    {
        std::array<BYTE, 0x40> dips;
        overlapped overlapped(readEvent.get());

        // start an asynchronous I/O operation
        auto readResult = file.read(dips.begin(), dips.end(), overlapped.get());
        if (readResult.get_result() != ERROR_IO_PENDING) as_hr(readResult).throw_if_failed();

        if (wait_for_any_object(signalInterrupt, readEvent).value() == signalInterrupt.get())
        {
            file.cancel(overlapped.get());
            return;
        }

        auto bytesRead = overlapped.get_num_bytes_read(file.get(), dword_milliseconds(0)).value();

        std::for_each(dips.begin(), dips.begin() + bytesRead, [](BYTE const & value) {
            wprintf_s(L"%02X\n", value);
            _flushall();
        });
    }
}

void PrintUsage(PCWSTR executable)
{
    std::wcout 
        << std::endl
        << L"usage: " << executable << L" <command>" << std::endl 
        << std::endl
        << L"Commands:" << std::endl
        << L"\tlisten - output all dip switch changes" << std::endl;
    
}

void osrtest(PCWSTR executable, PWSTR * argsBegin, PWSTR * argsEnd)
{
    if (argsBegin == argsEnd)
    {
        PrintUsage(executable);
        throw hresult_exception(E_FAIL);
    }

    if (0 == _wcsicmp(*argsBegin, L"listen"))
    {
        listen();
        return;
    }

    PrintUsage(executable);
    throw hresult_exception(E_FAIL);
}

int wmain(int argc, wchar_t ** argv)
{
    auto executable = argv;
    auto argBegin = argv + 1;
    auto argEnd = argv + argc;

    try
    {
        osrtest(*executable, argBegin, argEnd);
    }
    catch (hresult_exception const & hre)
    {
        wprintf_s(L"Error: Failed with 0x%08X\n", hre.error());
        return 1;
    }

    return 0;
}

