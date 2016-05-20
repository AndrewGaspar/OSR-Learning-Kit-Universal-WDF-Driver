/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID

#include <ntddk.h>
#include <wdf.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

//
// WDFDRIVER Events
//

EXTERN_C DRIVER_INITIALIZE DriverEntry;
EXTERN_C EVT_WDF_DRIVER_DEVICE_ADD DriverEvtDeviceAdd;
EXTERN_C EVT_WDF_OBJECT_CONTEXT_CLEANUP DriverEvtDriverContextCleanup;
