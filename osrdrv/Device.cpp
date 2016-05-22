/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DriverCreateDevice)
#endif


NTSTATUS
DriverCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    PAGED_CODE();

    OSRLogEntry();

    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    WDFDEVICE device;
    RETURN_IF_NT_FAILED(WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device));

    //
    // Get a pointer to the device context structure that we just associated
    // with the device object. We define this structure in the device.h
    // header file. DeviceGetContext is an inline function generated by
    // using the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro in device.h.
    // This function will do the type checking and return the device context.
    // If you pass a wrong object handle it will return NULL and assert if
    // run under framework verifier mode.
    //
    auto deviceContext = DeviceGetContext(device);
    
    //
    // Initialize the context.
    //
    deviceContext->PrivateDeviceData = 0;
    
    //
    // Create a device interface so that applications can find and talk
    // to us.
    //
    RETURN_IF_NT_FAILED(WdfDeviceCreateDeviceInterface(
        device,
        &GUID_DEVINTERFACE_OSR_FX2,
        NULL // ReferenceString
        ));

    //
    // Initialize the I/O Package and any Queues
    //
    RETURN_IF_NT_FAILED(DriverQueueInitialize(device));

    OSRLogExit();

    return STATUS_SUCCESS;
}


