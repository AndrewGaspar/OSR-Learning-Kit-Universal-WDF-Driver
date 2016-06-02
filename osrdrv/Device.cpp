/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "Precomp.h"
#include "Driver.h"

PASSIVE PAGED NTSTATUS DriverCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit)
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

    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = EvtOSRDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = EvtOSRD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = EvtOSRD0Exit;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

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
    *deviceContext = {};
    
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

PASSIVE PAGED NTSTATUS EvtOSRD0Entry(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE PreviousState)
{
    UNREFERENCED_PARAMETER((PreviousState));

    OSRLogEntry();

    auto context = DeviceGetContext(Device);

    if (context->DipSwitches)
    {
        RETURN_IF_NT_FAILED(context->DipSwitches.Start());
    }

    if (context->InData)
    {
        RETURN_IF_NT_FAILED(context->InData.Start());
    }

    if (context->OutData)
    {
        RETURN_IF_NT_FAILED(context->OutData.Start());
    }

    OSRLogExit();

    return STATUS_SUCCESS;
}

PASSIVE PAGED NTSTATUS EvtOSRD0Exit(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE TargetState)
{
    UNREFERENCED_PARAMETER((TargetState));

    OSRLogEntry();

    auto context = DeviceGetContext(Device);

    if (context->DipSwitches)
    {
        context->DipSwitches.Stop(WdfIoTargetCancelSentIo);
    }

    if (context->InData)
    {
        context->InData.Stop(WdfIoTargetCancelSentIo);
    }

    if (context->OutData)
    {
        context->OutData.Stop(WdfIoTargetCancelSentIo);
    }

    OSRLogExit();

    return STATUS_SUCCESS;
}

PASSIVE PAGED NTSTATUS EvtOSRDevicePrepareHardware(
    _In_ WDFDEVICE Device,
    _In_ WDFCMRESLIST ResourcesRaw,
    _In_ WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER((ResourcesRaw, ResourcesTranslated));

    PAGED_CODE();

    OSRLogEntry();

    auto context = DeviceGetContext(Device);

    if (!context->UsbDevice)
    {
        WDF_USB_DEVICE_CREATE_CONFIG createParams;
        WDF_USB_DEVICE_CREATE_CONFIG_INIT(&createParams, USBD_CLIENT_CONTRACT_VERSION_602);

        RETURN_IF_NT_FAILED(
            WdfUsbTargetDeviceCreateWithParameters(Device, &createParams, WDF_NO_OBJECT_ATTRIBUTES, &context->UsbDevice));

        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&configParams);
        
        RETURN_IF_NT_FAILED(
            WdfUsbTargetDeviceSelectConfig(context->UsbDevice, WDF_NO_OBJECT_ATTRIBUTES, &configParams));

        context->UsbInterface = configParams.Types.SingleInterface.ConfiguredUsbInterface;

        context->NumberConfiguredPipes = WdfUsbInterfaceGetNumConfiguredPipes(context->UsbInterface);

        if (context->NumberConfiguredPipes == 0)
        {
            RETURN_IF_NT_FAILED(USBD_STATUS_BAD_NUMBER_OF_ENDPOINTS);
        }

        for (UINT8 i = 0; i < context->NumberConfiguredPipes; i++)
        {
            WDF_USB_PIPE_INFORMATION pipeInfo;
            WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

            auto pipe = WdfUsbInterfaceGetConfiguredPipe(context->UsbInterface, i, &pipeInfo);

            OSRLoggingWrite(
                "AvailablePipe",
                TraceLoggingValue(pipeInfo.EndpointAddress, "EndpointAddress"),
                TraceLoggingUInt32(pipeInfo.PipeType, "PipeType"),
                TraceLoggingValue(pipeInfo.Interval, "Interval"),
                TraceLoggingValue(pipeInfo.SettingIndex, "SettingIndex"),
                TraceLoggingValue(pipeInfo.MaximumPacketSize, "MaximumPacketSize"),
                TraceLoggingValue(pipeInfo.MaximumTransferSize, "MaximumTransferSize"));

            switch (pipeInfo.EndpointAddress)
            {
            case DipSwitchEndpoint:
                context->DipSwitches = { pipeInfo, pipe };
                break;
            case DataOutEndpoint:
                context->OutData = { pipeInfo, pipe };
                break;
            case DataInEndpoint:
                context->InData = { pipeInfo, pipe };
                break;
            }
        }

        if (context->DipSwitches)
        {
            PFN_WDF_USB_READER_COMPLETION_ROUTINE interruptComplete = [](
                _In_ WDFUSBPIPE Pipe,
                _In_ WDFMEMORY Buffer,
                _In_ size_t NumBytesTransferred,
                _In_ WDFCONTEXT Context) 
            {
                UNREFERENCED_PARAMETER((Context, Pipe));

                if (NumBytesTransferred != 1)
                {
                    OSRLoggingWrite("InvalidDipSwitchReadSize", TraceLoggingLevel(TRACE_LEVEL_ERROR), TraceLoggingValue(NumBytesTransferred));
                    return;
                }

                auto value = *static_cast<BYTE*>(WdfMemoryGetBuffer(Buffer, nullptr));

                OSRLoggingWrite("DipSwitchValue", TraceLoggingLevel(TRACE_LEVEL_INFORMATION), TraceLoggingValue(value, "Value"));
            };

            WDF_USB_CONTINUOUS_READER_CONFIG readerConfig;
            //WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&readerConfig, OsrInterruptDipSwitchReadComplete, context, sizeof(BYTE));
            WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&readerConfig, interruptComplete, context, context->DipSwitches.Info.MaximumPacketSize);

            RETURN_IF_NT_FAILED(WdfUsbTargetPipeConfigContinuousReader(context->DipSwitches.Object, &readerConfig));
        }
        else
        {
            OSRLoggingWrite("DipSwitchesMissing", TraceLoggingLevel(TRACE_LEVEL_ERROR));
        }
    }

    OSRLogExit();

    return STATUS_SUCCESS;
}