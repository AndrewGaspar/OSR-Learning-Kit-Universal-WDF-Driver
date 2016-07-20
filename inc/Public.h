/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

#pragma once

#include <guiddef.h>

//
// Define an Interface Guid so that app can find the device and talk to it.
//

// {728c3fef-86fd-4f28-8953-cc8abe6263a2}
constexpr static GUID GUID_DEVINTERFACE_OSR_FX2 = 
    { 0x728c3fef,0x86fd,0x4f28,{0x89,0x53,0xcc,0x8a,0xbe,0x62,0x63,0xa2} };

#define DIP_SWITCHES_FILE L"DIP_SWITCHES"
