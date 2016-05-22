#pragma once

#include <evntrace.h>
#include <TraceLoggingProvider.h>

TRACELOGGING_DECLARE_PROVIDER(OSRDriverTraceProvider);

#define OSRLoggingWrite(eventName, ...) \
    TraceLoggingWrite(OSRDriverTraceProvider, eventName, __VA_ARGS__)

#define OSRContextInfo() \
    TraceLoggingWideString((__FUNCTIONW__), "Function"), \
    TraceLoggingWideString((__FILEW__), "File"), \
    TraceLoggingUInt32((__LINE__), "Line")

#define OSRLogEntry() \
    OSRLoggingWrite( \
        "FunctionEntry", \
        TraceLoggingLevel(TRACE_LEVEL_VERBOSE), \
        OSRContextInfo())

#define OSRLogExit() \
    OSRLoggingWrite( \
        "FunctionExit", \
        TraceLoggingLevel(TRACE_LEVEL_VERBOSE), \
        OSRContextInfo())