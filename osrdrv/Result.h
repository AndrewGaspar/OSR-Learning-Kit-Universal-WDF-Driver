#pragma once

#include <ntdef.h>
#include "TraceLogging.h"

#define REQUIRE_SEMICOLON (true)

#define RETURN_IF_NT_FAILED_MESSAGE(_StatusExpression, _TraceLevel, _Message) { \
    auto _status = (_StatusExpression); \
    if(!NT_SUCCESS(_status)) \
    { \
        OSRLoggingWrite("UnexpectedFailure", \
            TraceLoggingLevel(_TraceLevel), \
            TraceLoggingNTStatus(_status, "Status"), \
            TraceLoggingString(_Message, "Message"), \
            OSRContextInfo()); \
        return _status; \
    } \
} REQUIRE_SEMICOLON

#define RETURN_IF_NT_FAILED(_StatusExpression) RETURN_IF_NT_FAILED_MESSAGE(_StatusExpression, TRACE_LEVEL_INFORMATION, #_StatusExpression)

#define RETURN_IF_NT_FAILED_UNEXPECTED(_StatusExpression) RETURN_IF_NT_FAILED_MESSAGE(_StatusExpression, TRACE_LEVEL_ERROR, #_StatusExpression)