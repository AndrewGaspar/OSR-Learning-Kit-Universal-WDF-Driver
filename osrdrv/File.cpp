#include "Precomp.h"

#include "TraceLogging.h"
#include "File.h"
#include "Public.h"

PAGED PASSIVE VOID
EvtOSRDeviceFileCreate(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request,
    _In_ WDFFILEOBJECT FileObject)
{
    UNREFERENCED_PARAMETER((Device));

    PAGED_CODE();

    OSRLogEntry();

    auto fileName = WdfFileObjectGetFileName(FileObject);

    auto fileContext = GetFileContext(FileObject);

    *fileContext = {};

    OSRLoggingWrite("FileCreate", TraceLoggingUnicodeString(fileName, "FileName"));

    if (0 == _wcsicmp(fileName->Buffer, L"\\" DIP_SWITCHES_FILE))
    {
        fileContext->File = FileType::DipSwitches;
        WdfRequestComplete(Request, STATUS_SUCCESS);
    }
    else
    {
        WdfRequestComplete(Request, STATUS_FILE_INVALID);
    }


    OSRLogExit();
}
