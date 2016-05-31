/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#pragma once

#include "Public.h"

constexpr UINT8 DipSwitchEndpoint = 129;
constexpr UINT8 DataOutEndpoint = 6;
constexpr UINT8 DataInEndpoint = 136;

struct Pipe
{
    WDF_USB_PIPE_INFORMATION Info;
    WDFUSBPIPE Object;

    __forceinline operator bool() const
    {
        return !!Object;
    }

    __forceinline NTSTATUS Start()
    {
        return WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(Object));
    }

    __forceinline void Stop(WDF_IO_TARGET_SENT_IO_ACTION action)
    {
        WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(Object), action);
    }
};

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    WDFUSBDEVICE UsbDevice;
    WDFUSBINTERFACE UsbInterface;
    ULONG NumberConfiguredPipes;

    // dip switches
    Pipe DipSwitches;

    // data
    Pipe OutData;
    Pipe InData;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
EXTERN_C NTSTATUS DriverCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);
EXTERN_C EVT_WDF_DEVICE_PREPARE_HARDWARE EvtOSRDevicePrepareHardware;
EXTERN_C EVT_WDF_DEVICE_D0_ENTRY EvtOSRD0Entry;
EXTERN_C EVT_WDF_DEVICE_D0_EXIT EvtOSRD0Exit;