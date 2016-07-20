#pragma once

enum class FileType
{
    DipSwitches
};

struct FileContext
{
    FileType File;
};

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FileContext, GetFileContext);

EXTERN_C PAGED EVT_WDF_DEVICE_FILE_CREATE      EvtOSRDeviceFileCreate;
