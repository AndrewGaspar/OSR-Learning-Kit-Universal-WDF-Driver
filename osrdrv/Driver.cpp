/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

#include <ktl\scope.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DriverEvtDeviceAdd)
#pragma alloc_text (PAGE, DriverEvtDriverContextCleanup)
#endif

// {27d1e96e-78c4-41d7-aba4-9f9ca3379a15}
TRACELOGGING_DEFINE_PROVIDER(
    OSRDriverTraceProvider,
    "OSRTraceProvider",
    (0x27d1e96e, 0x78c4, 0x41d7, 0xab, 0xa4, 0x9f, 0x9c, 0xa3, 0x37, 0x9a, 0x15));

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    // Initialize TraceLogging
    TraceLoggingRegister(OSRDriverTraceProvider);

    auto unregisterLoggingOnFailure = ktl::make_scope_exit([]() {
        TraceLoggingUnregister(OSRDriverTraceProvider);
    });

    OSRLogEntry();

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = DriverEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config,
                           DriverEvtDeviceAdd
                           );

    RETURN_IF_NT_FAILED_UNEXPECTED(
        WdfDriverCreate(DriverObject, RegistryPath, &attributes, &config, WDF_NO_HANDLE));

    OSRLogExit();

    unregisterLoggingOnFailure.Dismiss();

    return STATUS_SUCCESS;
}

NTSTATUS
DriverEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    OSRLogEntry();

    RETURN_IF_NT_FAILED(DriverCreateDevice(DeviceInit));

    OSRLogExit();

    return STATUS_SUCCESS;
}

VOID
DriverEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();

    OSRLogEntry();

    TraceLoggingUnregister(OSRDriverTraceProvider);
}
