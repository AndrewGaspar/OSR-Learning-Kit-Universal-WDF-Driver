/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#pragma once

//
// WDFDRIVER Events
//

EXTERN_C PASSIVE INIT_CODE DRIVER_INITIALIZE DriverEntry;
EXTERN_C PASSIVE PAGED     EVT_WDF_DRIVER_DEVICE_ADD DriverEvtDeviceAdd;
EXTERN_C PASSIVE PAGED     EVT_WDF_OBJECT_CONTEXT_CLEANUP DriverEvtDriverContextCleanup;
