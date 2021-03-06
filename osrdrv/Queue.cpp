/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "Precomp.h"

#include "Queue.h"
#include "Driver.h"
#include "TraceLogging.h"
#include "Result.h"
#include "File.h"
#include "Device.h"

PASSIVE PAGED NTSTATUS DriverQueueInitialize(
    _In_ WDFDEVICE Device)
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    PAGED_CODE();

    OSRLogEntry();
    
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG ioctlQueueConfig;
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioctlQueueConfig, WdfIoQueueDispatchParallel);

    ioctlQueueConfig.EvtIoDeviceControl = DriverEvtIoDeviceControl;
    ioctlQueueConfig.EvtIoRead = EvtOSRDeviceRead;
    ioctlQueueConfig.EvtIoStop = DriverEvtIoStop;

    WDFQUEUE ioctlQueue;
    RETURN_IF_NT_FAILED(
        WdfIoQueueCreate(Device, &ioctlQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &ioctlQueue));

    OSRLogExit();

    return STATUS_SUCCESS;
}

UP_TO_DISPATCH NONPAGED VOID
EvtOSRDeviceRead(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t Length)
{
    UNREFERENCED_PARAMETER(Length);

    OSRLogEntry();

    auto device = WdfIoQueueGetDevice(Queue);
    auto deviceContext = DeviceGetContext(device);

    auto fileObject = WdfRequestGetFileObject(Request);
    auto fileContext = GetFileContext(fileObject);

    if (fileContext->File == FileType::DipSwitches)
    {
        auto status = [&]() {
            WDFMEMORY memory;
            RETURN_IF_NT_FAILED(WdfRequestRetrieveOutputMemory(Request, &memory));
            
            size_t size;
            WdfMemoryGetBuffer(memory, &size);

            RETURN_IF_NT_FAILED(WdfUsbTargetPipeFormatRequestForRead(
                deviceContext->DipSwitches.Object,
                Request,
                memory,
                nullptr));

            auto completionRoutine = [](WDFREQUEST Request, WDFIOTARGET, PWDF_REQUEST_COMPLETION_PARAMS Params, WDFCONTEXT) {
                OSRLogEntry();

                WdfRequestCompleteWithInformation(Request, Params->IoStatus.Status, Params->IoStatus.Information);

                OSRLogExit();
            };

            WdfRequestSetCompletionRoutine(Request, completionRoutine, nullptr);

            auto target = WdfUsbTargetPipeGetIoTarget(deviceContext->DipSwitches.Object);
            if (!WdfRequestSend(Request, target, WDF_NO_SEND_OPTIONS))
            {
                RETURN_IF_NT_FAILED(WdfRequestGetStatus(Request));
            }

            return STATUS_SUCCESS;
        }();

        if (!NT_SUCCESS(status))
        {
            WdfRequestComplete(Request, status);
        }
    }
    else
    {
        WdfRequestComplete(Request, STATUS_NOT_IMPLEMENTED);
    }

    OSRLogExit();
}

PASSIVE PAGED VOID DriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode)
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    OSRLogEntry();

    OSRLoggingWrite("IOCTL Request",
        TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
        TraceLoggingPointer(Queue),
        TraceLoggingPointer(Request),
        TraceLoggingValue(OutputBufferLength),
        TraceLoggingValue(InputBufferLength),
        TraceLoggingValue(IoControlCode));

    WdfRequestComplete(Request, STATUS_SUCCESS);

    OSRLogExit();
}

PASSIVE PAGED VOID DriverEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    OSRLogEntry();

    OSRLoggingWrite(
        "Stop Queue",
        TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
        TraceLoggingPointer(Queue),
        TraceLoggingPointer(Request),
        TraceLoggingValue(ActionFlags));

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it calls WdfRequestUnmarkCancelable
    //   (if the request is cancelable) and either calls WdfRequestStopAcknowledge
    //   with a Requeue value of TRUE, or it calls WdfRequestComplete with a
    //   completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //
    //   Before it can call these methods safely, the driver must make sure that
    //   its implementation of EvtIoStop has exclusive access to the request.
    //
    //   In order to do that, the driver must synchronize access to the request
    //   to prevent other threads from manipulating the request concurrently.
    //   The synchronization method you choose will depend on your driver's design.
    //
    //   For example, if the request is held in a shared context, the EvtIoStop callback
    //   might acquire an internal driver lock, take the request from the shared context,
    //   and then release the lock. At this point, the EvtIoStop callback owns the request
    //   and can safely complete or requeue the request.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge with
    //   a Requeue value of FALSE.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time.
    //
    // In this case, the framework waits until the specified request is complete
    // before moving the device (or system) to a lower power state or removing the device.
    // Potentially, this inaction can prevent a system from entering its hibernation state
    // or another low system power state. In extreme cases, it can cause the system
    // to crash with bugcheck code 9F.
    //

    OSRLogExit();
}

