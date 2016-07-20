/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#pragma once

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    ULONG PrivateDeviceData;  // just a placeholder

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

EXTERN_C PASSIVE PAGED NTSTATUS DriverQueueInitialize(
    _In_ WDFDEVICE hDevice);

//
// Events from the IoQueue object
//
EXTERN_C PAGED EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL    DriverEvtIoDeviceControl;
EXTERN_C PAGED EVT_WDF_IO_QUEUE_IO_STOP              DriverEvtIoStop;
EXTERN_C NONPAGED EVT_WDF_IO_QUEUE_IO_READ           EvtOSRDeviceRead;

