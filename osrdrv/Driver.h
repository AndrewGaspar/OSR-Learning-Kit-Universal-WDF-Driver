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
#include <usb.h>
#include <usbdlib.h>
#include <wdfusb.h>

#include "Device.h"
#include "Queue.h"
#include "TraceLogging.h"
#include "Result.h"

//
// WDFDRIVER Events
//

EXTERN_C DRIVER_INITIALIZE DriverEntry;
EXTERN_C EVT_WDF_DRIVER_DEVICE_ADD DriverEvtDeviceAdd;
EXTERN_C EVT_WDF_OBJECT_CONTEXT_CLEANUP DriverEvtDriverContextCleanup;
