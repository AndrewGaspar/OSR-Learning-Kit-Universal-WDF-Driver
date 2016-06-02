#pragma once

#include <ntdef.h>
#include "TraceLogging.h"

#define REQUIRE_SEMICOLON (true)

#define RETURN_IF_NT_FAILED_LOG(_StatusExpression, _TraceLevel, ...) { \
    auto _status = (_StatusExpression); \
    if(!NT_SUCCESS(_status)) \
    { \
        OSRLoggingWrite("UnexpectedFailure", \
            TraceLoggingLevel(_TraceLevel), \
            TraceLoggingNTStatus(_status, "Status"), \
            __VA_ARGS__, \
            OSRContextInfo()); \
        return _status; \
    } \
} REQUIRE_SEMICOLON

#define RETURN_IF_NT_FAILED_EXPRESSION(_Expression, _TraceLevel, ...) RETURN_IF_NT_FAILED_LOG(_Expression, _TraceLevel, TraceLoggingString(#_Expression, "Expression"), __VA_ARGS__)

#define RETURN_IF_NT_FAILED_EXPECTED(_StatusExpression, ...) RETURN_IF_NT_FAILED_EXPRESSION(_StatusExpression, TRACE_LEVEL_INFORMATION, __VA_ARGS__)

#define RETURN_IF_NT_FAILED(_StatusExpression, ...) RETURN_IF_NT_FAILED_EXPRESSION(_StatusExpression, TRACE_LEVEL_ERROR, __VA_ARGS__)